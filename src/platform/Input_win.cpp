#include <platform/Input_impl.h>
#include <Windows.h>
#include <utils/Debug.h>

using namespace CoreInput;

BYTE input[256];
MouseState mouseState;
Vector2 windowSize;
Vector2 hWindowSize;


void CoreInput::Initialize(int wWidth, int wHeight){
    windowSize.x = (float) wWidth;
    windowSize.y = (float) wHeight;
    hWindowSize.x = ((float) wWidth) / 2.0f;
    hWindowSize.y = ((float) wHeight) / 2.0f;
}


void CoreInput::UpdateKeyboardState() {
    GetKeyboardState(input);
}


bool CoreInput::IsKeyPressed(KeyCode key) {
    return ((input[(int)key] & 0x80) > 0) ? true : false;
}


void CoreInput::UpdateMousePosition(int x, int y) {
    mouseState.pos.x = x;
    mouseState.pos.y = y;
}


void CoreInput::UpdateMouseScroll(int value) {
    mouseState.scroll = (value / WHEEL_DELTA);
}


Vector2 CoreInput::GetWindowMousePosition() {
    return mouseState.pos;
}


Vector2 CoreInput::GetNDCMousePosition() {
    return Vector2{
        ( mouseState.pos.x - hWindowSize.x ) / hWindowSize.x,
        ( mouseState.pos.y - hWindowSize.y ) / hWindowSize.y
    };
    // float x = ( mouseState.pos.x - hWindowSize.x );
    // float y = ( mouseState.pos.y - hWindowSize.y );
    // return Vector2{x, -y};
}


int CoreInput::GetMouseScroll() {
    return mouseState.scroll;
}
