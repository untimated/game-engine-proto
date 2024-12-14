#ifndef CORE_GLOBALS_H
#define CORE_GLOBALS_H

#include <core/Physics.h>
#include <core/GameObject.h>
#include <core/SceneGraph.h>
#include <core/DebugDraw.h>
#include <unordered_map>
#include <string>

/*
 * Header:  CoreGlobals.h
 * Impl:    GameResources.cpp
 *          GameObject.cpp
 *          EngineCore.cpp
 * Purpose: Store, define references of global variables or data structures 
 * Author:  Michael Herman
 * */

typedef Node2D* (*FactoryFunctionType)(Node2D*);

namespace CoreGlobals {
    extern std::unordered_map<std::string, GameObject::Node2D*> nodes;
    extern std::unordered_map<std::string, std::vector<GameObject::Node2D*>> _nodes;
    extern std::unordered_map<std::string, GameResource::Resource*> resources;
    extern std::unordered_map<std::string, GameResource::Material*> materials;
    extern std::unordered_map<std::string, GameResource::Material*> _materials;
    extern std::unordered_map<std::string, GameResource::Texture*> textures;
    extern std::unordered_map<std::string, GameResource::Texture*> _textures;
    extern std::unordered_map<std::string, GameResource::Shader*> shaders;
    extern std::unordered_map<std::string, GameResource::Shader*> _shaders;
    extern std::unordered_map<std::string, GameResource::Font*> fonts;
    extern std::unordered_map<std::string, GameResource::Font*> _fonts;

    extern std::unordered_map<std::string, FactoryFunctionType> gameTypesFactory;

    extern unsigned long nodeLastId;
    extern unsigned long emptyLastId;
    extern unsigned long spriteLastId;
    extern unsigned long cameraLastId;
    extern unsigned long textLastId;

    extern SceneGraph::Scene* activeScene;
    extern CorePhysics::World* physicsWorld;

    extern std::string PROJECT_BASE_PATH;
    const std::string RESOURCE_BASE_PATH = "resources";
    const std::string SHADERS_BASE_PATH = "shaders";
    const std::string ASSETS_BASE_PATH = "assets";
    const std::string SCENE_EXTENSION = ".scene.json";
    const std::string MATERIAL_EXTENSION = ".material.json";
    const std::string TEXTURE_EXTENSION = ".texture.json";
    const std::string SHADER_EXTENSION = ".shader.json";
    const std::string FONT_EXTENSION = ".font.json";
}
 
#endif
