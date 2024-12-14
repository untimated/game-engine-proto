#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

/*
 * Header:  NONE
 * Impl:    EngineParser.cpp
 * Purpose: Standalone meta program to identify things related to engine user script
 * Author:  Michael Herman
 * */

using namespace std;

enum class TokenType {
    OPEN_BRACE,
    CLOSE_BRACE,
    OPEN_PAREN,
    CLOSE_PAREN,
    SEMI_COLON,

    ASTERISK,

    UNKNOWN,
    IDENTIFIER,
    KEYWORD,
    USING_KEYWORD,
    OPERATOR,
    NUMBER,
    STRING,
    END,
};

struct Token {
    TokenType type = TokenType::UNKNOWN;
    char *word = nullptr;
    int length = 0;
};

struct Tokenizer {
    char *at;
};

struct FieldMember {
    Token type;
    Token name;
    Token value;
    bool isPointer = false;
};

struct FileContent {
    char *content;
    int size;
};

// Forward Definitions

FileContent OpenFile(const char *filename);
bool        WriteToFile(std::string content, std::string path);
string      GetFileNameFromPath(const char* filePath);
bool        IsNumeric(char ch);
bool        IsAlpha(char ch);
const char* PrintTokenType(TokenType type);
void        PrintToken(Token &token);
string      PrintExposedStruct(Token &structName, std::vector<FieldMember> &fields);
bool        TokenCompare(Token &token, const char* cmp);
bool        IsWhitespace(Tokenizer &tokenizer);
void        ParseSingleComments(Tokenizer &tokenizer);
void        ParseMultiComments(Tokenizer &tokenizer);
void        ParseStrings(Tokenizer &tokenizer, Token &token);
void        EatWhitespaces(Tokenizer &tokenizer);
Token       GetToken(Tokenizer &tokenizer);
bool        IsNextTokenMatch(Tokenizer tokenizer, TokenType match);
FieldMember ParseField(Tokenizer &tokenizer);


FileContent OpenFile(const char *filename) {
    FileContent result = {nullptr, 0};

    fstream file;
    file.open(filename, ios::in | ios::binary);
    if(file.fail() || !file.is_open()) {
        cout << "Cannot open file at " <<  filename << endl;
        return result;
    }

    // Check file size
    file.seekg(0, ios_base::end);
    int size = file.tellg();
    file.seekg(0, ios_base::beg);

    cout << "=========================" << endl;
    cout << "Buffer size : " << size << endl;

    // Read to buffer
    char *buffer = (char *) malloc(sizeof(char) * size);
    if(!buffer) {
        cout << "Fail allocating buffer " << endl;
        return result;
    }
    file.read(buffer, size);
    cout << "Read count : " << file.gcount() << endl;
    if(file.bad()) {
        cout << "Fail to read file content" << endl;
        return result;
    }

    file.close();

    buffer[size] = '\0';
    result.content = buffer;
    result.size = file.gcount();

    cout << "=========================" << endl;

    return result;
}


// bool WriteToFile(std::string content, std::string path, std::string fileName) {
bool WriteToFile(std::string content, std::string path) {
    cout << "\nWriting content to " << path << endl;
    fstream file(path.c_str(), std::ofstream::out | std::ofstream::binary);
    file.write(content.c_str(), content.length());
    if(file.bad()) {
        cout <<  "Fail to write content to file at" << path << endl;
    }
    file.close();
    return true;
}


std::string GetFileNameFromPath(const char* filePath) {
    std::string path(filePath);
    int start = path.find_last_of('/');
    int end = path.find_last_of('.');
    start++;
    return path.substr(start, end - start);
}


bool IsNumeric(char ch) {
    return (ch >= '0') && (ch <= '9');
}


bool IsAlpha(char ch) {
    return ((ch >= 'a') && (ch <= 'z')) || 
           ((ch >= 'A') && (ch <= 'Z'));
}


const char* PrintTokenType(TokenType type) {
    switch(type){
        case TokenType::OPEN_BRACE : return "open_brace";
        case TokenType::CLOSE_BRACE : return "close_brace";
        case TokenType::OPEN_PAREN : return "open_parentheses";
        case TokenType::CLOSE_PAREN : return "close_parentheses";
        case TokenType::SEMI_COLON : return "semi_colon";
        case TokenType::ASTERISK : return "asterisk";
        case TokenType::IDENTIFIER : return "identifier";
        case TokenType::KEYWORD : return "keyword";
        case TokenType::END : return "eof";
        case TokenType::OPERATOR : return "operator";
        case TokenType::STRING : return "string";
        case TokenType::NUMBER : return "number";
        case TokenType::UNKNOWN : return "unknown";
        default : return "unknown" ;
    }
}


void PrintToken(Token &token) {
    for(int i = 0; i < token.length; i++) {
        cout << token.word[i];
    }
    printf(" : %s (%i)\n", PrintTokenType(token.type), token.length);
}


std::string PrintExposedStruct(Token &structName, std::vector<FieldMember> &fields) {
    std::string sName(structName.word, structName.length);
    std::string result = "ReflectedStruct ";
    result += sName + "_fields[] = { \n";
    for(auto &field : fields) {
        std::string name(field.name.word, field.name.length);
        std::string type(field.type.word, field.type.length);
        std::string value(field.value.word, field.value.length);
        result += "  {";
        result += "meta_" + type + (field.isPointer ? "_p" : "") + ", ";
        result += "\"" + name + "\", ";
        result += "offsetof(" + sName + "::" + sName + ", " + name +  ")";
        result += "},\n";
    }
    result += "};";
    cout << result;
    return result;
}


bool TokenCompare(Token &token, const char* cmp) {
    if(token.length != strlen(cmp)) return false;

    bool isEqual = true;
    for(int i = 0; i < token.length; i++) {
        if(token.word[i] != cmp[i]) {
            isEqual = false;
            break;
        }
    }
    return isEqual;
}


bool IsWhitespace(Tokenizer &tokenizer) {
    if( tokenizer.at[0] == ' ' || 
        tokenizer.at[0] == '\n' ||
        tokenizer.at[0] == '\t' ||
        tokenizer.at[0] == '\r'
    ) {
        return true;
    }else {
        return false;
    }
}


void ParseSingleComments(Tokenizer &tokenizer) {
    while(tokenizer.at[0] != '\0'){
        if(tokenizer.at[0] == '\n') {
            break;
        }
        tokenizer.at++;
    }
}


void ParseMultiComments(Tokenizer &tokenizer) {
    while(tokenizer.at[0] != '\0'){
        if(tokenizer.at[0] == '*' && tokenizer.at[1] == '/') {
            tokenizer.at += 2;
            break;
        }
        tokenizer.at++;
    }
}


void ParseStrings(Tokenizer &tokenizer, Token &token) {
    token.type = TokenType::STRING;
    tokenizer.at++;
    while(tokenizer.at[0] != '\0'){
        if(tokenizer.at[0] == '"') {
            break;
        }
        tokenizer.at++;
    }
}


void EatWhitespaces(Tokenizer &tokenizer) {
    while(*tokenizer.at != '\0') {
        if(IsWhitespace(tokenizer)) {
            tokenizer.at++;
            continue;
        }
        else if(tokenizer.at[0] == '/' && tokenizer.at[1] == '/') {
            ParseSingleComments(tokenizer);
        }
        else if(tokenizer.at[0] == '/' && tokenizer.at[1] == '*') {
            ParseMultiComments(tokenizer);
        }
        else {
            break;
        }
    }
}


Token GetToken(Tokenizer &tokenizer) {
    EatWhitespaces(tokenizer);

    Token token = {};
    token.word = tokenizer.at;

    switch(tokenizer.at[0]){
        case '(' : { token.type = TokenType::OPEN_PAREN;} break;
        case ')' : { token.type = TokenType::CLOSE_PAREN;} break;
        case '{' : { token.type = TokenType::OPEN_BRACE;} break;
        case '}' : { token.type = TokenType::CLOSE_BRACE;} break;
        case '=' : { token.type = TokenType::OPERATOR;} break;
        case '*' : { token.type = TokenType::ASTERISK;} break;
        case ';' : { token.type = TokenType::SEMI_COLON;} break;
        case '\0' : { token.type = TokenType::END;} break;
        case '"' : 
        {
            ParseStrings(tokenizer, token);
        } break;
        break;
        case '#' : { 
            token.word = tokenizer.at;
            token.type = TokenType::KEYWORD;
            while(tokenizer.at[0] != '\0') {
                if(IsWhitespace(tokenizer)){
                    break;
                }
                tokenizer.at++;
            }
            token.length = tokenizer.at - token.word;
            return token;
        } break;
        default : 
        {
            if(IsAlpha(tokenizer.at[0])) {
                token.word = tokenizer.at;
                while(tokenizer.at[0] != '\0') {
                    if(IsAlpha(tokenizer.at[0]) || IsNumeric(tokenizer.at[0]) || tokenizer.at[0] == '_') {
                        tokenizer.at++;
                    } else {
                        break;
                    }
                }
                token.length = tokenizer.at - token.word;

                if(
                    TokenCompare(token, "int") ||
                    TokenCompare(token, "float") ||
                    TokenCompare(token, "double") ||
                    TokenCompare(token, "char")
                ) {
                    token.type = TokenType::KEYWORD;
                } else {
                    token.type = TokenType::IDENTIFIER;
                }

                return token;
            }
            else if(IsNumeric(tokenizer.at[0])) {
                token.word = tokenizer.at;
                token.type = TokenType::NUMBER;
                while(tokenizer.at[0] != '\0') {
                    if( tokenizer.at[0] == '.' || 
                        tokenizer.at[0] == 'f' ||
                        IsNumeric(tokenizer.at[0])
                    ) {
                        tokenizer.at++;
                    }else{
                        break;
                    }
                }
                token.length = tokenizer.at - token.word;
                return token;
            }
        } 
        break;
    }

    tokenizer.at++;
    token.length = tokenizer.at - token.word;
    return token;
}


// Check token without moving the tokenizer
bool IsNextTokenMatch(Tokenizer tokenizer, TokenType match) {
    Token next = GetToken(tokenizer);
    return (next.type == match);
};


FieldMember ParseField(Tokenizer &tokenizer) {
    cout << "Parsing field" << endl;
    FieldMember currentField;
    while(tokenizer.at[0] != '\0'){
        Token token = GetToken(tokenizer);
        // PrintToken(token);
        if(token.type == TokenType::SEMI_COLON) break;

        switch(token.type) {
            case TokenType::KEYWORD : 
            {
                currentField.type = token;
            }break;
            case TokenType::IDENTIFIER : 
            {
                if(TokenCompare(token, "SKIP")) {
                    while(GetToken(tokenizer).type != TokenType::SEMI_COLON) {}
                    break;
                } 
                currentField.name = token;
            }break;
            case TokenType::ASTERISK : 
            {
                currentField.isPointer = true;
            }break;
            case TokenType::OPERATOR : 
            {
                if (TokenCompare(token, "=")) {
                    Token value = GetToken(tokenizer);
                    currentField.value = value;
                }
            }break;
            default : break;
        } 
    }
    // printf("-- Type = %.*s \n", currentField.type.length, currentField.type.word);
    // printf("-- Name = %.*s \n", currentField.name.length, currentField.name.word);
    // if(currentField.value.type != TokenType::UNKNOWN) {
    //     printf("-- Value = %.*s \n", currentField.value.length, currentField.value.word);
    // }
    // printf("-- IsPointer = %s \n", currentField.isPointer ? "true" : "false");
    return currentField;
}


int main(int argc, char* argv[]) {
    cout << "number of argument : " << argc << endl;

    if(argc < 2) {
        cout << "No input path argument supplied" << endl;
        return 0;
    }

    const char *filePath = argv[1];
    const char *fileOutputPath = argv[2];
    const char *StructKeyword = "REFLECT";
    std::string fileName = GetFileNameFromPath(filePath);

    FileContent file = OpenFile(filePath);
    
    if(!file.content) {
        cout << "Exiting" << endl;
        return 0;
    }

    Tokenizer tokenizer = {
        file.content
    };

    bool stopParse = false;

    while(*tokenizer.at != '\0' && !stopParse) {
        Token token = GetToken(tokenizer);
        // PrintToken(token);
        if(token.type == TokenType::END) {
            break;
        }
        switch(token.type) {
            case TokenType::IDENTIFIER : 
            {
                if(TokenCompare(token, StructKeyword)) {
                    // cout << "\n----------- reflection identifier is found, begin parsing struct" << endl;

                    Token structKeyword = GetToken(tokenizer);
                    if(!TokenCompare(structKeyword, "struct")) break;

                    Token structName = GetToken(tokenizer);
                    Token begin = GetToken(tokenizer);

                    // PrintToken(structKeyword);
                    // PrintToken(structName);

                    std::vector<FieldMember> fields;
                    if(begin.type == TokenType::OPEN_BRACE) {
                        do {
                            FieldMember field = ParseField(tokenizer);
                            fields.push_back(field);
                        } while(!IsNextTokenMatch(tokenizer, TokenType::CLOSE_BRACE));
                    }

                    // cout << "----------- End Parsing\n" << endl;

                    std::string result = PrintExposedStruct(structName, fields);
                    WriteToFile(result, fileOutputPath + fileName + ".generated.h");

                    stopParse = true; // stop we reached our goal for current parser goal
                }
            } break;
            case TokenType::KEYWORD : 
            {
                if(TokenCompare(token, "#define")) {
                    // Simply skip any macro definition for now
                    while(tokenizer.at[0] != '\0') {
                        if(tokenizer.at[0] == '\n') break;
                        tokenizer.at++;
                    }
                }
            } break;
            default : 
            {

            } break;
        }
    }

    free(file.content);
    file.content = nullptr;

    return 0;
}
