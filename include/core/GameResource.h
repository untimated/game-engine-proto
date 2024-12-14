#ifndef GAMERESOURCE_H
#define GAMERESOURCE_H

#include <platform/IO.h>
#include <platform/FontLoader.h>
#include <core/Math.h>
#include <unordered_map>
#include <string>
#include <any>


/*
 * Header:  GameResource.h
 * Impl:    GameResource.cpp
 * Purpose: Subsystem of engine core for game resources 
 * Author:  Michael Herman
 * */


struct GraphicsResource {
    enum Type {
        UNKNOWN,
        SHADER_RESOURCE,
        TEXTURE_RESOURCE,
        VERTEX_RESOURCE,
        CONSTANT_BUFFER_RESOURCE,
        MATERIAL_RESOURCE
    } type = Type::UNKNOWN;
    void *buffer = nullptr;
};

namespace GameResource {

    struct Resource {
        std::string id;
        std::string name;
    };

    
    // Textures
    struct Texture : Resource {
        // std::string id;
        // std::string name;
        std::string filePath;
        CoreMath::Vector2 dimension;
        GraphicsResource resource;
    };


    // Shader
    enum ShaderParamType {
        INTEGER, //RENAME
        FLOATING,
        VEC2,
        VEC3,
        VEC4
    };
    struct ShaderParamData {
        ShaderParamType dataType;
        std::any value; // you can optimize this using union
    };
    typedef std::unordered_map<std::string, ShaderParamData> ShaderParams;
    struct Shader : Resource {
        // std::string id;
        // std::string name;
        std::string filePath;
        ShaderParams parameterMeta; // serialization meta
        GraphicsResource resource;
    };

    
    // Materials
    struct Material : Resource {
        // std::string id;
        // std::string name;
        Texture *mainTexture;
        Shader *shader;
        ShaderParams shaderParameters; // holds actual data
        GraphicsResource resource;
    };


    // Fonts
    struct Font : Resource {
        // std::string id;
        // std::string name;
        std::string filePath;
        FontLoader::Font *fontResource;
    };

}

#endif
