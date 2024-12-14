#ifndef ENGINE_PLATFORM_API_H
#define ENGINE_PLATFORM_API_H

/*
 * Header:  EnginePlatformAPI.h
 * Impl:    EnginePlatform.cpp
 * Purpose: Platform specific functions that are commonly needed by higher layers
 * Author:  Michael Herman
 * */

#include <Windows.h>
#include <string>


namespace EnginePlatformAPI {
    static VOID OpenConsole();

    std::string GetProjectBasePath();

    POINT GetWindowDimension();

    VOID GetWindowDimension(LONG &x, LONG &y);

    VOID SetGameFPS(UINT fps);
    UINT GetTargetFPS();

    UINT GetScreenDPI();

    UINT GetGlobalFrameCount();
}

#endif
