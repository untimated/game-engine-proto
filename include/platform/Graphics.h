#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <core/GameObject.h>
#include <core/DebugDraw.h>

/*
 * Header:  Graphics.h
 * Impl:    Graphics_d3d.cpp
 *          Graphics_metal.cpp (not yet made)
 * Purpose: Interface bridging platform & platform independent layer
 * Author:  Michael Herman
 * */


namespace Graphics {

    bool CreateShader(GameResource::Shader *shader);
    bool RemoveShader(GameResource::Shader *shader);

    bool CreateTexture(GameResource::Texture *texture);
    bool RemoveTexture(GameResource::Texture *texture);

    bool CreateMaterial(GameResource::Material *material);
    bool RemoveMaterial(GameResource::Material *material);

    bool CreateGeometry(GameObject::Sprite *sprite);
    bool RemoveGeometry(GameObject::Sprite *sprite);

    bool CreateGeometry(GameObject::Text *text);
    bool RemoveGeometry(GameObject::Text *text);

    bool CreateGeometry(DebugDraw::Text *text);
    bool RemoveGeometry(DebugDraw::Text *text);

    void Draw(GameObject::Sprite *sprite);
    void Draw(GameObject::AnimatedSprite *animatedSprite);
    void Draw(GameObject::Text *text);
    void Draw(GameObject::Camera *camera);
    void Draw(CoreGeometry::BoundingRect &aabb);

    void Draw(DebugDraw::Text *text);


    bool UpdateMaterialParameters(GameResource::Material **material);
    Vector2 GetScreenDimension();
    void UpdateViewProjectionMatrix(GameObject::Camera *camera);
    bool UpdateTexture(GraphicsResource &textureResource, GraphicsResource &vertexResource, void *data, uint32_t width, uint32_t height);
    bool UpdateStaticTexture(GraphicsResource &textureResource, void *data, uint32_t boxW, uint32_t boxH);

    // void SyncGeometryAndBuffer();
}

#endif
