#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <core/Math_impl.h>
#include <core/GameResource.h>
#include <vector>

/*
 * Header:  Geometry.h
 * Impl:    Geometry.cpp
 * Purpose: Geometric library and structures provider
 * Author:  Michael Herman
 * */

// TODO: Improve this module from physics engine,
// make mulitple intersection test, shapes inheritance etc


using namespace CoreMath;

namespace CoreGeometry {
    struct Shape {

    };

    struct BoundingRect {
        union {
            struct {
                float minX;
                float minY;
                float maxX;
                float maxY;
            };
            struct{
                Vector2 min;
                Vector2 max;
            };
            Vector4 minmax;
        } bound;
    };

    BoundingRect CreateAABB(
        std::vector<Vector4> vertices,
        Matrix transform = CoreMath::IdentityMatrix()
        );
    void UpdateAABB(
        BoundingRect *rect,
        std::vector<Vector4> vertices,
        Matrix transform = CoreMath::IdentityMatrix()
        );

    // bool Intersect(BoundingShape *b1, BoundingShape *b2);
    bool Intersect(BoundingRect *b1, BoundingRect *b2);

}

#endif
