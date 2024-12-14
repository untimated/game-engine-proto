#ifndef CORE_MATH_H
#define CORE_MATH_H

#define PI 3.14159265

namespace CoreMath {

    union Vector2 {
        struct {
            float x;
            float y;
        };
        struct {
            float r;
            float g;
        };
        float f[2];
    };

    union Vector3 {
        struct {
            float x;
            float y;
            float z;
        };
        struct {
            float r;
            float g;
            float b;
        };
        float f[3];
    };

    union Vector4 {
        struct {
            float x;
            float y;
            float z;
            float w;
        };
        struct {
            float r;
            float g;
            float b;
            float a;
        };
        Vector2 xy;
        Vector3 xyz;
        float f[4];
    };

    union Matrix {
        float f[16];
        float m[4][4];
        Vector4 r[4];
        struct {
            float m11, m12, m13, m14;
            float m21, m22, m23, m24;
            float m31, m32, m33, m34;
            float m41, m42, m43, m44;
        };
    };

}

#endif
