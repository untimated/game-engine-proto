#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#include <Windows.h>
#include <string>
#include <iostream>

// using namespace std;

namespace Debug {

    inline void LogPrint(std::string &msg) {msg += "\n";};

    template <typename... Variadic>
    void LogPrint(std::string &msg, float arg, Variadic... args) {
        // msg += to_string(arg);
        char buff[20];
        snprintf(buff, 20, "%f", arg);
        msg += buff;
        msg += " ";
        LogPrint(msg, args...);
    };
    template <typename... Variadic>
    void LogPrint(std::string &msg, std::string arg, Variadic... args) {
        msg += arg + " ";
        LogPrint(msg, args...);
    };
    template <typename... Variadic>
    void Logger(Variadic... args) {
        std::string output;
        LogPrint(output, args...);
        // std::cout << output << std::endl;
        OutputDebugStringA(output.c_str());
    };


    inline void LogPrintW(std::wstring &msg) {msg += L"\n";};
    template <typename... Variadic>
    void LogPrintW(std::wstring &msg, float arg, Variadic... args) {
        msg += std::to_wstring(arg) + L" ";
        LogPrintW(msg, args...);
    };
    template <typename... Variadic>
    void LogPrintW(std::wstring &msg, std::wstring arg, Variadic... args) {
        msg += arg + L" ";
        LogPrintW(msg, args...);
    };
    template <typename... Variadic>
    void LoggerW(Variadic... args) {
        std::wstring output;
        LogPrintW(output, args...);
        OutputDebugStringW(output.c_str());
    };

    // TODO: temporary
    std::wstring ConvertStringToW(std::string str);
}

#endif
