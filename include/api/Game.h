#ifndef GAME_H
#define GAME_H

#include <api/Engine.h>

using namespace Engine;
using namespace CoreInput;
using namespace CoreMath;

/*
 * Header:  Game.h
 * Impl:    Game.cpp
 * Purpose: Game DLL entry point
 * Author:  Michael Herman
 * */

namespace Game {

    bool Init();
    void Start(SceneGraph::Scene *activeScene);
    void Update(uint32_t fps, double deltaTime);
    void Shutdown();

}

#endif
