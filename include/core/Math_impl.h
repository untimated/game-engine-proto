#ifndef CORE_MATH_IMPL_H
#define CORE_MATH_IMPL_H

#include <core/Math.h>
#include <string>

#define PI 3.14159265

namespace CoreMath {

    Vector2 CreateVector2(float x, float y);
    Vector3 CreateVector3(float x, float y, float z);
    Vector4 CreateVector4(float x, float y, float z, float w);
    Vector2 VectorAdd(Vector2 &v1, Vector2 &v2);
    Vector3 VectorAdd(Vector3 &v1, Vector3 &v2);
    Vector4 VectorAdd(Vector4 &v1, Vector4 &v2);
    Vector4 VectorScale(Vector4 &v1, float scale);
    Vector2 VectorMul(Vector2 &v1, float multiplier);
    Vector2 VectorSubtract(Vector2 &v1, Vector2 &v2);

    float Dot(const Vector4 &v1, const Vector4 &v2);
    Vector4 Cross(const Vector4 &v1, const Vector4 &v2);
    Vector4 Normalize(const Vector4 &v);

    Matrix CreateMatrix(float m[4][4]);
    Matrix CreateTranslationMatrix(const Vector2 &offset);
    Matrix CreateScaleMatrix(const Vector2 &scale);
    Matrix CreateZRotationMatrix(float angle);
    Matrix IdentityMatrix();
    Matrix ViewSpaceMatrix(Vector4 pos, Vector4 up);
    Matrix ProjectionSpaceMatrix();
    Matrix Multiply(const Matrix &A, const Matrix &B);
    Vector4 Multiply(const Matrix &A, const Vector4 &V);
    Matrix Transpose(const Matrix &A);

    std::string VectorToString(const Vector4 &v);
    std::string VectorToString(const Vector3 &v);
    std::string VectorToString(const Vector2 &v);
    std::string MatrixToString(const Matrix &A);
    void PrintMatrix(const Matrix &A);

}

#endif
