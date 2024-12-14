#ifndef INPUT_IMPL_H
#define INPUT_IMPL_H

#include <platform/Input.h>

/*
 * Header:  Input_impl.h
 * Impl:    Input.cpp
 * Purpose: Handle keyboard and mouse input from OS
 * Author:  Michael Herman
 * */

namespace CoreInput {

    void Initialize(int wWidth, int wHeight);

    void UpdateKeyboardState();
    bool IsKeyPressed(KeyCode key);

    void UpdateMousePosition(int x, int y);
    void UpdateMouseScroll(int value);
    CoreMath::Vector2 GetWindowMousePosition();
    CoreMath::Vector2 GetNDCMousePosition();
    int GetMouseScroll();

}


#endif
