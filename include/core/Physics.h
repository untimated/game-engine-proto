#ifndef PHYSICS_H
#define PHYSICS_H

// #include <core/Physics.fwd.h>
#include <core/GameObject.h>

/*
 * Header:  Physics.h
 * Impl:    Physics.cpp
 * Purpose: 
 * Author:  Michael Herman
 * */


namespace CorePhysics {

    enum ColliderType{
        BOX_COLLIDER
    };

    struct Collider {
        ColliderType type;
        GameObject::Transform2D transform;
        Vector2 velocity;
        bool visible = false;
        GameObject::Empty *owner;
    };

    struct World {
        std::vector<Collider*> colliders;
    };

    struct BoxCollider : Collider {
        CoreGeometry::BoundingRect AABB;
    };

    struct RayCast {};

    bool WorldMake();
    bool WorldDestroy();

    BoxCollider* CreateBoxCollider(
        // GameObject::Transform2D &transform,
        GameObject::Empty *gameObject,
        CoreGeometry::BoundingRect &boundingRect
        );
    void RegisterCollider(Collider *collider);
    void Step(double deltaTime);


}


#endif
