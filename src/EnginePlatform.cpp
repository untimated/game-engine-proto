#include <EnginePlatform.h>
#include <EnginePlatformAPI.h>
#include <cstdio>
#include <iostream>


HWND hwnd;
UINT TargetFPS      = 30;
UINT FrameCount     = 0;
DOUBLE TargetPeriod = 1.0f / TargetFPS;
FILE *fp;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int cmdShow){
    // OpenConsole();
    const wchar_t *CLASS_NAME = L"EnginePlatform";
    const wchar_t *WINDOW_TITLE = L"GameWindow"; 
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = CLASS_NAME;
    wc.hInstance = hInstance;
    if(!RegisterClassW(&wc)) {
        DWORD err = GetLastError();
        Debug::Logger("Fail register class , error code : ", err);
    }

    hwnd = CreateWindowExW(
        0,
        CLASS_NAME, 
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    if(hwnd == NULL){
        DWORD err = GetLastError();
        Debug::Logger("Fail create window handle, error code : ", err);
        return 0;
    }
    
    ShowWindow(hwnd, cmdShow);
    UpdateWindow(hwnd);

    // GetWindowSize
    POINT wDim = EnginePlatformAPI::GetWindowDimension();
    
    // Init Game Input
    CoreInput::Initialize(wDim.x, wDim.y);

    // Init Font
    FontLoader::Initialize();

    // Init Graphic
    HRESULT d3dInitResult = Graphics_D3D::Initialize(hwnd, wDim);
    if(FAILED( d3dInitResult )){
        MessageBoxEx(hwnd, "Graphic initializations fail", "Error", MB_OK, 0);
        Debug::Logger("Fail init direct3d, show empty window");
        PlatformShutdown();
        return 0;
    }
    Debug::Logger("Graphic initialized");

    // Init Core
    if(!EngineCore::Start()) {
        Debug::Logger("Engine Core Fail to Start, aborting application");
        PlatformShutdown();
        return 0;
    };
    Debug::Logger("Engine Core initialized");
    EngineCore::SetGameBasePath(projectPath);

    // float bg[4] = {0.0f, 0.0f, 0.5, 1.0f};
    float bg[4] = {0.1f, 0.1f, 0.1, 1.0f};
    bool done = false;
    MSG msg;

    LARGE_INTEGER tick;
    LARGE_INTEGER frequency;
    UINT fps = TargetFPS;
    DOUBLE deltaTime = TargetPeriod;
    assert(QueryPerformanceCounter(&tick));
    assert(QueryPerformanceFrequency(&frequency));

    while (!done)
    {
        if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if(msg.message == WM_QUIT){
            Debug::Logger("Quit Program");
            PlatformShutdown();
            done = true;
            continue;
        }

        CoreInput::UpdateKeyboardState();
        Graphics_D3D::UpdateShaderGlobalConstants();
        Graphics_D3D::ClearBackground(bg);
        EngineCore::UpdateAndRender(fps, deltaTime);
        Graphics_D3D::Present();
        CoreInput::UpdateMouseScroll(0);

        //TODO: should we use accumulate frame instead of sleep ?
        CalculateFPSAndDeltaTime(tick, frequency, fps, deltaTime);
        if(fps > TargetFPS) {
            DWORD timeToSleep = static_cast<DWORD>(( TargetPeriod - deltaTime ) * 1000);
            // Debug::Logger("time to sleep:", timeToSleep, "dt:", deltaTime * 1000, "target:", TargetPeriod * 1000 );
            if(timeToSleep > 0) {
                Sleep(timeToSleep);
            }
        }
        FrameCount = FrameCount >= ( TargetFPS - 1 ) ? 0 : ++FrameCount;
    }

    return 0;
}


static VOID CalculateFPSAndDeltaTime(LARGE_INTEGER &tick, LARGE_INTEGER &frequency, UINT &fps, DOUBLE &deltaTime) {
    LONGLONG start = tick.QuadPart;
    if(!QueryPerformanceCounter(&tick)) {
        Debug::Logger("fail to get end tick, skip getting new fps");
        return;
    }
    assert(QueryPerformanceFrequency(&frequency));
    LONGLONG elapsed = tick.QuadPart - start;      // ticks per frame
    fps = frequency.QuadPart / elapsed;            // frame per second = (ticks per second) * (1 / (ticks per frame) )
    deltaTime = (double) elapsed / frequency.QuadPart;
}


static VOID PlatformShutdown() {
    EngineCore::Shutdown();
    Graphics_D3D::Shutdown();
    FontLoader::Shutdown();
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_KEYDOWN : 
        {
#if DEBUG==1
            if(wParam == VK_ESCAPE){
                SendMessage(hwnd, WM_CLOSE, 0, 0);
            }
#endif
            break;
            // int previousState = lParam & (1 << 31) ? 1 : 0;
            // keyState.keyCode = (UINT) wParam;
            // keyState.state = true;
        }
        break;
        case WM_KEYUP :
        {
            // int previousState = lParam & (1 << 31) ? 1 : 0;
            // keyState.keyCode = (UINT) wParam;
            // keyState.state = false;
        }
        break;
        case WM_MOUSEWHEEL :
        {
            SHORT fwKeys = GET_KEYSTATE_WPARAM(wParam);
            SHORT zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            // Debug::Logger("ZDelta", std::to_string(zDelta));
            CoreInput::UpdateMouseScroll(zDelta);
        }
        break;
        case WM_MOUSEMOVE :
        {
            CoreInput::UpdateMousePosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            break;
        }
        case WM_CREATE : 
        {
            Debug::Logger("Window created");
            break;
        }
        case WM_SIZE : 
        {
            Debug::Logger("Window resized");
            break;
        }
        case WM_CLOSE : 
        {
            Debug::Logger("Window closing");
            DestroyWindow(hwnd);
            break;
        }
        case WM_DESTROY : 
        {
            Debug::Logger("Window destroyed");
            PostQuitMessage(0);
            break;
        }
        default: return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}


static VOID OpenConsole () {
    AllocConsole();
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONOUT$", "w", stdin);
    std::cout << "console allocated" << std::endl;
}


/*
 * Implementations of exposed apis
 * */

std::string EnginePlatformAPI::GetProjectBasePath(){
    char base[300]; 
    GetCurrentDirectory(300, base);
    std::string path = std::string(base);
    Debug::Logger("Project Path: ", path.c_str());
    return path;
}



POINT EnginePlatformAPI::GetWindowDimension() {
    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);
    return POINT {
       windowRect.right - windowRect.left,
       windowRect.bottom - windowRect.top,
    };
}


VOID EnginePlatformAPI::GetWindowDimension(LONG &x, LONG &y) {
    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);
    x=windowRect.right - windowRect.left;
    y=windowRect.bottom - windowRect.top;
}


VOID EnginePlatformAPI::SetGameFPS(uint32_t fps) {
    TargetFPS = fps;
    TargetPeriod = 1.0f / TargetFPS;
    Debug::Logger("Changing FPS From : ", TargetFPS, ", To : ", fps, ", Target Period: ", TargetPeriod);
}


UINT EnginePlatformAPI::GetTargetFPS() {
    return TargetFPS;
}


UINT EnginePlatformAPI::GetScreenDPI() {
    return GetDpiForWindow(hwnd);
}


UINT EnginePlatformAPI:: GetGlobalFrameCount() {
    return FrameCount;
}

