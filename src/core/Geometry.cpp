#include <core/Geometry.h>

using namespace CoreGeometry;

BoundingRect CoreGeometry::CreateAABB(std::vector<Vector4> vertices, Matrix transform) {
    std::vector<Vector4> _vertices;
    for(Vector4 &vertex : vertices) {
        _vertices.push_back(CoreMath::Multiply(transform, vertex));
    }

    float maxX = _vertices[0].x; 
    float minX = _vertices[0].x;
    float maxY = _vertices[0].y;
    float minY = _vertices[0].y;
    for(Vector4 v : _vertices) {
        maxX = std::max(v.x, maxX);
        maxY = std::max(v.y, maxY);
        minX = std::min(v.x, minX);
        minY = std::min(v.y, minY);
    }

    BoundingRect aabb;
    aabb.bound = {minX, minY, maxX, maxY};
    return aabb;
}


void CoreGeometry::UpdateAABB(BoundingRect *rect, std::vector<Vector4> vertices, Matrix transform) {
    std::vector<Vector4> _vertices;
    for(Vector4 &vertex : vertices) {
        _vertices.push_back(CoreMath::Multiply(transform, vertex));
    }

    float maxX = _vertices[0].x; 
    float minX = _vertices[0].x;
    float maxY = _vertices[0].y;
    float minY = _vertices[0].y;
    for(Vector4 v : _vertices) {
        maxX = std::max(v.x, maxX);
        maxY = std::max(v.y, maxY);
        minX = std::min(v.x, minX);
        minY = std::min(v.y, minY);
    }

    rect->bound = {minX, minY, maxX, maxY};
}


bool CoreGeometry::Intersect(BoundingRect *b1, BoundingRect *b2) {
    bool lIntersect = (b1->bound.minX <= b2->bound.minX) && (b1->bound.maxX >= b2->bound.minX);
    bool rIntersect = (b1->bound.minX <= b2->bound.maxX) && (b1->bound.maxX >= b2->bound.maxX);
    bool tIntersect = (b1->bound.minY <= b2->bound.maxY) && (b1->bound.maxY >= b2->bound.maxY);
    bool bIntersect = (b1->bound.minY <= b2->bound.minY) && (b1->bound.maxY >= b2->bound.minY);

    return (lIntersect || rIntersect) && (tIntersect || bIntersect);

}
