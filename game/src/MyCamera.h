#ifndef MYCAMERA_H
#define MYCAMERA_H

#include <api/Engine.h>

/*
 * Header:  MyCamera.h
 * Impl:    MyCamera.cpp
 * Purpose: Camera Movement
 * Author:  Michael Herman
 * */


namespace MyCamera {

    REFLECT struct MyCamera {
        SKIP GameObject::Camera parent;
        float velocity = 1.0f;
        int count = 0;
    };

    GAME_CODE_HEADER_DEFINTION()

}

#endif
