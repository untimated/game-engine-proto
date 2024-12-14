#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <core/Physics.fwd.h>
#include <core/GameResource.h>
#include <core/Geometry.h>
#include <string>
#include <api/Meta.h>
#include <platform/IO.h>
// #include <variant>

/*
 * Header:  GameObject.h
 * Impl:    GameObject.cpp
 * Purpose: Subsystem of engine core for high level game objects creation
 *          such as sprites, empty, spatials, camera, lines, circles
 * Author:  Michael Herman
 * */


using namespace CoreMath;
using namespace GameResource;


namespace GameObject {

    /*
     * General
     * */

    enum Type {
        NODE2D = 0,
        EMPTY = 1,
        SPRITE = 2,
        ANIMATED_SPRITE = 3,
        CAMERA = 4,
        TEXT = 5,
        LINE = 6
    };

    struct Geometry2D {
        CoreGeometry::BoundingRect AABB;
        std::vector<Vector4> vertices;
        GraphicsResource mesh;
        bool showBoundingRect = false;
    };

    struct Transform2D {
        Vector4 pos;
        Vector4 worldPos;
        Vector2 scale;
        float rotation;
        Matrix Local;
        Matrix World;
    };

    /*
     * Nodes
     * */

    struct Node2D {
        Type type;
        std::string id;
        std::string name;
        std::string tag;
        std::vector<Node2D*> children;
        Node2D* parent;
        int zIndex = 0;
        std::vector<MetaField> meta;
        struct {
            void (*Start)(Node2D*) = nullptr;
            void (*Update)(Node2D*, uint32_t fps, float deltaTime) = nullptr;
            void (*Shutdown)(Node2D*) = nullptr;
            void (*Serialize)(Node2D*) = nullptr;
            void (*DeSerialize)(Node2D*) = nullptr;
        } behavior;
    };

    struct Empty {
        Node2D attribute;
        Transform2D transform;
        CorePhysics::Collider *collider = nullptr;
    };

    struct Sprite {
        Node2D attribute;
        Transform2D transform;
        CorePhysics::Collider *collider = nullptr;
        Geometry2D geometry;
        GameResource::Material *material;
    };

    struct AnimatedSprite {
        //TODO: to support multiple animations 
        // we have to refactor all into specific AnimatedSprite functions
        // and no longer we can rely on Sprite reuse
        // For example add these attribute instead of Sprite sprite;
        //    Node2D attribute;
        //    Transform2D transform;
        //    Geometry2D geometry;
        
        Sprite sprite;
        Vector2 frameDimension;
        Vector2 frameDimensionNormalized;
        uint32_t pitch;
        uint32_t currentFrame;
        uint32_t totalFrames;
        uint32_t fps;
        uint32_t localFrameCount = 0;
        bool isPlay = true;
    };

    struct Camera {
        Node2D attribute;
        Transform2D transform;
        Geometry2D geometry;
        Vector4 up = { 0.0, 1.0f, 0.0f, 0.0f };
        Matrix view;
        // Physics::Collider collider;
    };

    struct Text {
        Node2D attribute;
        Transform2D transform;
        Geometry2D geometry;
        GameResource::Font *font;

        RGBA *surfaceBuffer = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t size = 16;
        std::string text;

        GraphicsResource textureResource;
        GraphicsResource constantBuffers;
    };

    // struct UI;
    // struct Panel;
    // struct TextLabel;
    // struct TextBox;

}

#endif
