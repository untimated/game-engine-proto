#ifndef GAMEOBJECT_IMPL_H
#define GAMEOBJECT_IMPL_H

#include <core/GameObject.h> 

namespace GameObject {
    std::string GenerateGameObjectID(Type type);

    Node2D* CreateNode2D(std::string name);
    // std::vector<Node2D*> GetNodesByName(std::string name);
    
    Empty* CreateEmptyObject(
        std::string name,
        std::string tag,
        CoreMath::Vector2 pos,
        CoreMath::Vector2 scale,
        float rotation,
        std::string id = ""
        );

    Sprite* CreateSprite(
        std::string name,
        std::string tag,
        GameResource::Material *material,
        Vector2 position,
        Vector2 scale,
        float rot,
        std::string id = ""
        );
    bool FreeSpriteGeometryResource(Sprite **sp);

    AnimatedSprite* CreateAnimatedSprite(
        std::string name,
        std::string tag,
        GameResource::Material *material,
        unsigned int startFrame,
        unsigned int fps,
        Vector2 frameDimension,
        Vector2 position,
        Vector2 scale,
        float rot,
        std::string id = ""
        );

    Camera* CreateCamera(
        std::string name,
        std::string tag,
        const Vector2 pos,
        std::string id = ""
        );

    Text* CreateText(
        std::string text,
        std::string name,
        GameResource::Font *font,
        const Vector2 pos,
        uint32_t size,
        std::string id = ""
        );

}

#endif
