#include <utils/RUID.h>
#include <utils/Debug.h>
#include <cstdlib>
#include <cmath>

unsigned int randomNumericSeed = 1100;
unsigned int randomAlphaSeed = 12;

char ALPHABET[26] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

static void reset() {
    randomNumericSeed = randomNumericSeed < 100000 ? randomNumericSeed : 1000;
    randomAlphaSeed = randomAlphaSeed < 100 ? randomAlphaSeed : 12;
}

static unsigned int LinearCongruentialGenerator(
    unsigned int seed,
    const unsigned int modulus = std::pow(2, 11),
    const unsigned int multiplier = 2,
    const unsigned int increment = 1
){
    return (( multiplier * seed ) + increment) % modulus;
}


wchar_t GetResourceSignature(Signature signature){
    switch(signature) {
        case MATERIAL : return L'M';
        case TEXTURE : return L'T';
        case SHADER : return L'S';
        case SCENE : return L'G';
        case CAMERA : return L'C';
        case FONT : return L'F';
        default : return L'X';
    }
}


std::string RUID::GenerateResourceUID(Signature signature, unsigned int seedNoise){
    reset();
    randomNumericSeed = LinearCongruentialGenerator(randomNumericSeed + seedNoise);
    std::srand(randomNumericSeed);
    long long rn = ( rand() % 100000 ) + 100000;
    std::string randomNumeric = std::to_string(rn);

    std::string randomAlpha = "";
    for(int i = 0; i < 4; i++){
        randomAlphaSeed = LinearCongruentialGenerator(randomAlphaSeed + seedNoise, sizeof(ALPHABET)-1, 2, 1);
        randomAlpha += ALPHABET[randomAlphaSeed];
    }

    std::string ruid = "";
    ruid += GetResourceSignature(signature);
    ruid += "-" + randomAlpha + "" + randomNumeric;

    return ruid;
}


wchar_t* RUID::GenerateResourceUIDW(Signature signature){
    randomNumericSeed = LinearCongruentialGenerator(randomNumericSeed);
    std::srand(randomNumericSeed);
    long long rn = ( rand() % 100000 ) + 100000;
    std::wstring randomNumeric = std::to_wstring(rn);

    std::wstring randomAlpha = L"";
    for(int i = 0; i < 4; i++){
        randomAlphaSeed = LinearCongruentialGenerator(randomAlphaSeed, sizeof(ALPHABET)-1, 2, 1);
        randomAlpha += ALPHABET[randomAlphaSeed];
    }

    std::wstring ruid = L"";
    ruid += GetResourceSignature(signature);
    ruid += L"-" + randomAlpha + L"" + randomNumeric;

    int bufferSize = ruid.length() + 1;
    wchar_t *result = (wchar_t*) malloc(bufferSize * sizeof(wchar_t));

    if(!result) return nullptr;

    wcscpy_s(result, bufferSize, ruid.c_str());

    return result;
}
