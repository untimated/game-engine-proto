#ifndef DEBUGDRAW_H
#define DEBUGDRAW_H

#include <core/GameResource.h>
#include <cstdint>

// #include <string>
// #include <variant>

/*
 * Header:  DebugDraw.h
 * Impl:    DebugDraw.cpp
 * Purpose: Debugging purpose drawings such as shapes and texts
 *          it should be rendered after main graph is completed.
 *          - Debug objects should not be saved in any sort of level file, draw only through code.
 * Author:  Michael Herman
 * */

using namespace CoreMath;

namespace DebugDraw {

    enum Type {
        TEXT,
        CIRCLE,
        LINE
    };


    struct DebugObject {
        Type type;
        std::string id;
        std::string name;
    };

    extern std::vector<DebugObject*> drawables;
    extern std::unordered_map<std::string, DebugObject*> debugObjects;
    DebugObject GetDebugObject(std::string name);

    /*
     * Debug Text
     * */

    enum TextAlignment {
        ALIGN_LEFT = 0,
        ALIGN_RIGHT = 2
    };
    struct Text {
        DebugObject attribute;

        Vector4 pos;
        Vector2 scale;
        float rotation;

        RGBA *surfaceBuffer = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t boxW = 0;
        uint32_t boxH = 0;
        uint32_t size = 16;
        float scaleFactor = 1.0f;
        TextAlignment alignment;
        CoreMath::Vector2 margin;

        GraphicsResource vertexResource;
        GraphicsResource textureResource;
        GraphicsResource constantBuffers;
    };


    // Possible optimization
    // struct DebugText {
    //     Text key;
    //     Text value;
    // };

    // Text* CreateText(std::string text, Vector2 pos, std::string name);
    Text* CreateText(std::string text, std::string name, TextAlignment options, const Vector2 margin = Vector2{100.0f, 100.0f});
    void SetText(std::string name, std::string text);


    // do not call this from game code
    void DrawPass();
    void Shutdown();

}

#endif
