#ifndef INPUT_H
#define INPUT_H

#include <core/Math.h>

/*
 * Header:  Input.h
 * Impl:    Input.cpp
 * Purpose: Handle keyboard and mouse input from OS
 * Author:  Michael Herman
 * */

using namespace CoreMath;


extern unsigned char input[256];

namespace CoreInput {
    enum class KeyCode {
        MOUSE_LEFT = 0x1,
        MOUSE_RIGHT = 0x2,

        KEY_0 = 0x30,
        KEY_1 = 0x31,

        KEY_A = 0x41,
        KEY_D = 0x44,
        KEY_S = 0x53,
        KEY_W = 0x57,

        KEY_Q = 0x51,
        KEY_E = 0x45,

        KEY_LEFT = 0x25, 
        KEY_UP = 0x26, 
        KEY_RIGHT = 0x27, 
        KEY_DOWN = 0x28, 
    };

    struct MouseState {
        CoreMath::Vector2 pos;
        int scroll;
    };

}


#endif
