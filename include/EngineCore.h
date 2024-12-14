#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#include <core/CoreGlobals.h>
#include <core/SceneGraph.h>
#include <core/GameResource_impl.h>
#include <core/GameObject_impl.h>
#include <core/GameLoader.h>
#include <core/Physics.h>
#include <core/DebugDraw.h>
#include <utils/Debug.h>
#include <platform/Input.h>
#include <platform/Graphics.h>
#include <platform/IO.h>
#include <api/Game.h>

/*
 * Header:  EngineCore.h
 * Impl:    EngineCore.cpp
 * Purpose: Core system entry, used by EnginePlatform
 * Author:  Michael Herman
 * */

static bool InitDefaultTextureAndMaterial();

namespace EngineCore {
    bool Start();
    void UpdateAndRender(uint32_t fps, double deltaTime);
    void Shutdown();
    void SetGameBasePath(const char* path);
}
 
#endif
