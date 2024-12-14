#ifndef MYSPRITE_H
#define MYSPRITE_H

#include <api/Engine.h>

/*
 * Header:  MySprite.h
 * Impl:    MySprite.cpp
 * Purpose: Sprite
 * Author:  Michael Herman
 * */


namespace MySprite {

    REFLECT struct MySprite {
        SKIP GameObject::Sprite parent;
        float velocity = 1.0f;
        int count = 0;
    };

    GAME_CODE_HEADER_DEFINTION();

}

#endif
