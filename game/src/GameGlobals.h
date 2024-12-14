#ifndef GAME_GLOBALS_H
#define GAME_GLOBALS_H

#include <api/Engine.h>

/*
 * Header:  GameGlobals.h
 * Impl:    -
 * Purpose: Game DLL global var holder
 * Author:  Michael Herman
 * */

namespace GameGlobals {

    extern Engine::API          *GameAPI;
    extern SceneGraph::Scene    *scene;

}

#endif
