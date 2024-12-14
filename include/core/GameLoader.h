#ifndef GAME_LOADER_H
#define GAME_LOADER_H

#include <core/SceneGraph.h>
#include <core/GameResource.h>
#include <string>

/*
 * Header:  GameLoader.h
 * Impl:    GameLoader.cpp
 * Purpose: Loader for various GameResource/Objects,
 *          Mainly for load and unserialize level file
 * Author:  Michael Herman
 * */


namespace GameLoader {

    bool LoadLevelFromFile(std::string filePath);
    bool SaveLevelToFile(SceneGraph::Scene *scene, std::string dir = "", std::string fileName = "");

    bool SaveGameResourceToFile(GameResource::Material *material, std::string fileName= "", std::string dir = "");
    bool SaveGameResourceToFile(GameResource::Shader *shader, std::string fileName = "", std::string dir = "");
    bool SaveGameResourceToFile(GameResource::Texture *texture, std::string fileName = "", std::string dir = "");
    bool SaveGameResourceToFile(GameResource::Font *font, std::string fileName = "", std::string dir = "");

    bool LoadGameResourcesFromDirectory(std::string dir = "");

}

#endif
