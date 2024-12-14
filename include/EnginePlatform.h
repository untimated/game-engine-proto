#ifndef ENGINE_PLATFORM_H
#define ENGINE_PLATFORM_H

/*
 * Header:  EnginePlatform.h
 * Impl:    EnginePlatform.cpp
 * Purpose: Platform System Entry
 * Author:  Michael Herman
 * */

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#ifndef TOSTRING
#define TOSTRING(x) STRINGIFY(x)
#endif

#ifdef PROJECT_PATH
const char* projectPath = TOSTRING(PROJECT_PATH);
#else
const char* projectPath = ".";
#endif

#include <EngineCore.h>
#include <Windows.h>
#include <Windowsx.h>
#include <platform/Graphics_d3d.h>
#include <platform/Input_impl.h>
#include <utils/Debug.h>

static VOID CalculateFPSAndDeltaTime(LARGE_INTEGER &tick, LARGE_INTEGER &frequency, UINT &fps, DOUBLE &deltaTime);
static VOID PlatformShutdown();
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static VOID OpenConsole();

#endif
