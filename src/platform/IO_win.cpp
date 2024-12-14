#include <platform/IO.h>
#include <Windows.h>
#include <utils/Debug.h>


IO::FileBuffer IO::OpenAndReadFile(std::string path) {
    IO::FileBuffer file = {nullptr, 0};
    HANDLE fileHandle = CreateFileA(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if(fileHandle == INVALID_HANDLE_VALUE) {
        WORD err = GetLastError();
        Debug::Logger("IO::", "Error opening file, err code :", err);
        file.buffer = nullptr;
        return file;
    }
    
    DWORD fileSize = GetFileSize(fileHandle, NULL);
    if(!fileSize){
        WORD err = GetLastError();
        Debug::Logger("IO::", "Error opening file, err code :", err);
        file.buffer = nullptr;
        return file;
    }

    Debug::Logger("IO::", "File handle opened, path : \n", path, ", size :", fileSize);

    file.buffer = (char*) malloc(fileSize + 1);
    Debug::Logger("IO::", "allocated file.buffer size :", sizeof(*file.buffer));
    if(!ReadFile(fileHandle, file.buffer, fileSize, &file.bufferSize, NULL)) {
        WORD err = GetLastError();
        Debug::Logger("IO::", "file read fail, err code : ", err);
        file.buffer = nullptr;
        return file;
    }

    // important to add null terminated
    file.buffer[fileSize] = '\0';

    Debug::Logger("IO::","content :\n", file.buffer);
    Debug::Logger("IO::","bytesRead :", file.bufferSize);

    if(!CloseHandle(fileHandle)) { 
        WORD err = GetLastError();
        Debug::Logger("IO::", "file close handle fail, err code : ", err);
        free(file.buffer);
        file.buffer = nullptr;
        return file;
    }

    Debug::Logger("IO::", "File handle closed \n");
    return file ;

}


std::vector<std::string> IO::ListDirFiles(std::string pattern) {
    std::vector<std::string> result;

    WIN32_FIND_DATAA meta;
    HANDLE fileHandle = FindFirstFile(pattern.c_str(), &meta);

    if(fileHandle == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        Debug::Logger("IO::", "Error finding file, err code :", err);
        return result;
    }
    result.push_back(meta.cFileName);

    while(FindNextFile(fileHandle, &meta)){
        Debug::Logger("File Found :", meta.cFileName);
        result.push_back(meta.cFileName);
    }

    if(GetLastError() == ERROR_NO_MORE_FILES) {
        Debug::Logger("IO::", "no more files found");
    }

    return result;

}


bool IO::SaveFile(IO::FileBuffer *file, std::string path) {
    HANDLE fileHandle = CreateFileA(
        path.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if(fileHandle == INVALID_HANDLE_VALUE) {
        WORD err = GetLastError();
        Debug::Logger("IO::", "Error opening file, err code :", err);
        return false;
    }

    Debug::Logger("IO::", "File handle for writing is ready");

    DWORD bytesWritten = 0;
    if(!WriteFile(fileHandle, file->buffer, file->bufferSize, &bytesWritten, NULL)){
        WORD err = GetLastError();
        Debug::Logger("IO::", "Error writing file, err code :", err);
        return false;
    }

    if(!CloseHandle(fileHandle)) { 
        WORD err = GetLastError();
        Debug::Logger("IO::", "file close handle fail, err code : ", err);
        return file;
    }

    Debug::Logger("IO::", "Finish writing file", path ,"byteswritten :", bytesWritten);

    return true;

}


IO::LibHandler IO::LoadLib(std::string path) {
    HMODULE hModule = LoadLibraryA(path.c_str());
    if(!hModule) {
        Debug::Logger("Fail to open lib: ", path);
        return nullptr;
    }
    return hModule;
}


void IO::FreeLib(LibHandler *libHandler) {
    FreeLibrary(reinterpret_cast<HMODULE>(libHandler));
}


IO::LibFunction IO::GetLibFunction(void *libHandler, std::string functionName) {
    HMODULE hModule = reinterpret_cast<HMODULE>(libHandler);
    using returnType = long long (*)();
    FARPROC func = GetProcAddress(hModule, functionName.c_str());
    if(!func) Debug::Logger("Cannot find function named", functionName);
    return reinterpret_cast<LibFunction>(func);
}


