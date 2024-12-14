#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include <core/GameResource_impl.h>
#include <core/GameObject_impl.h>


/*
 * Header:  SceneGraph.h
 * Impl:    SceneGraph.cpp
 * Purpose: Structure Game Objects for rendering goals, 
 *          such as culling, and group transformations.
 * Author:  Michael Herman
 * */


using namespace GameObject;
using namespace GameResource;

namespace SceneGraph {

    struct Scene {
        std::string id;
        std::string name;
        std::string filePath;
        std::vector<Node2D*> drawable; // to be drawn objects after frustum intersection test
        GameObject::Camera *activeCamera;
        Node2D *sceneRoot;
        // std::vector<Node2D*> sceneObjects;
    };

    void Init();
    std::string GenerateSceneID();
    Node2D* GetSceneRoot(Scene **scene);
    std::vector<Node2D*> GetSceneNodesByName(Scene **scene, std::string name);
    bool AttachTo(Node2D *parent, Node2D *child);

    Scene* CreateScene(
        std::string name,
        Camera *activeCamera,
        Node2D *root = nullptr,
        std::string id = ""
        );
    Scene* GetSceneByName(std::string name);

    void InitPass(Scene *scene);
    void UpdatePass(Scene *scene, unsigned int fps, double deltaTime);
    void ShutdownPass(Scene *scene);
    void DrawPass(Scene *scene);
    // void DrawPass(Scene *debugdraw);
    void SortSceneDrawable(std::vector<Node2D*> &drawable);

}


#endif
