#include <utils/Debug.h>

// used in graphics_d3d
std::wstring Debug::ConvertStringToW(std::string str){
    std::wstring converted = L"";

#if OS == WIN && DEBUG == 1

    int bufferSize = str.size() + 1;
    wchar_t *ws = new wchar_t[bufferSize];
    int res = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        str.c_str(),
        bufferSize,
        ws,
        bufferSize
        );
    if(res != bufferSize || res == 0) {
        Logger("Error", GetLastError());
        return L"Fail Conversion";
    }
    converted = std::wstring(ws);
    delete[] ws;

#elif OS == MAC && DEBUG == 1

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring converted = converter.from_bytes(str);

#endif

    return converted;
}

