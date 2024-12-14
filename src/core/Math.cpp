#include <core/Math_impl.h>
#include <cmath>
#include <utils/Debug.h>


// TODO: need further monitoring whether pass by value is efficient for this structures

using namespace CoreMath;
using namespace std;
using namespace Debug;


Vector2 CoreMath::CreateVector2(float x, float y) {
    return Vector2{x,y};
}


Vector3 CoreMath::CreateVector3(float x, float y, float z) {
    return Vector3 {x, y, z};
}


Vector4 CoreMath::CreateVector4(float x, float y, float z, float w) {
    return Vector4 {x, y, z, w};
}


Vector2 CoreMath::VectorAdd(Vector2 &v1, Vector2 &v2) {
    Vector2 result = {
        v1.x + v2.x,
        v1.y + v2.y,
    };
    return result;
}


Vector4 CoreMath::VectorAdd(Vector4 &v1, Vector4 &v2) {
    Vector4 result = {
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z,
        v1.w + v2.w,
    };
    return result;
}


Vector4 CoreMath::VectorScale(Vector4 &v1, float scale) {
    return Vector4 {
        v1.x * scale,
        v1.y * scale,
        v1.z * scale,
        v1.w * scale
    };
}


Vector2 CoreMath::VectorMul(Vector2 &v1, float multiplier) {
    return Vector2 {
        v1.x * multiplier,
        v1.y * multiplier,
    };
}


Vector2 CoreMath::VectorSubtract(Vector2 &v1, Vector2 &v2) {
    Vector2 result = {
        v1.x - v2.x,
        v1.y - v2.y,
    };
    return result;
}


float CoreMath::Dot(const Vector4 &v1, const Vector4 &v2) {
    float result = 
         (v1.x * v2.x) +
         (v1.y * v2.y) +
         (v1.z * v2.z) +
         (v1.w * v2.w);
    return result;
}


Vector4 CoreMath::Cross(const Vector4 &v1, const Vector4 &v2) {
    // x   y   z   
    //--------------
    // Vx, Vy, Vz  
    // Wx, Wy, Wz
    Vector4 result = {
        ( v1.y * v2.z ) - ( v1.z * v2.y ),
        ( v1.z * v2.x ) - ( v1.x * v2.z ),
        ( v1.x * v2.y ) - ( v1.y * v2.x ),
        0.0f
    };

    return result;
}


Vector4 CoreMath::Normalize(const Vector4 &v) {
    float length = std::sqrt( (v.x * v.x) + (v.y * v.y) + (v.z * v.z) );
    Vector4 result = {
        v.x / length,
        v.y / length,
        v.z / length,
        v.w / length,
    };
    return result;
}


/*
 * Matrices
 * */


Matrix CoreMath::CreateMatrix(float m[4][4]){
    Matrix mat;
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            mat.m[i][j] = m[i][j];
        }
    }
    return mat;
}


Matrix CoreMath::CreateTranslationMatrix(const Vector2 &offset) {
    Matrix mat = {
        1.0f, 0.0f, 0.0f, offset.x,
        0.0f, 1.0f, 0.0f, offset.y,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return mat;
}


Matrix CoreMath::CreateScaleMatrix(const Vector2 &scale) {
    Matrix mat = {
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return mat;
}


Matrix CoreMath::CreateZRotationMatrix(float angle) {
    float rad = angle * (PI / 180);
    Matrix mat = {
        cos(rad), sin(rad), 0.0f, 0.0f,
        -sin(rad), cos(rad),  0.0f, 0.0f,
        0.0f,       0.0f,    1.0f, 0.0f,
        0.0f,       0.0f,    0.0f, 1.0f
    };
    return mat;
}


Matrix CoreMath::IdentityMatrix() {
    Matrix mat = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return mat;
}


Matrix CoreMath::ViewSpaceMatrix(Vector4 pos, Vector4 up) {
    Vector4 w = Vector4{0.0f, 0.0f, 0.0f - pos.z, 0.0f};
    Vector4 u = Normalize(Cross(w, up));
    Vector4 v = Normalize(Cross(u, w));
    Matrix mat = {
        //u   v    w    q
        u.x, v.x, w.x, -pos.x,
        u.y, v.y, w.y, -pos.y,
        u.z, v.z, w.z, pos.z,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return mat;
}


Matrix CoreMath::Multiply(const Matrix &A, const Matrix &B) {
    Matrix result;
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            float sum = 0.0f;
            for(int k = 0; k < 4; k++){
                float a = A.m[i][k];
                float b = B.m[k][j];
                sum += (a * b);
            }
            result.m[i][j] = sum;
        }
    }
    return result;
}


Vector4 CoreMath::Multiply(const Matrix &A, const Vector4 &B) {
    Vector4 result = {
        Dot(A.r[0], B),
        Dot(A.r[1], B),
        Dot(A.r[2], B),
        Dot(A.r[3], B)
    };
    return result;
}


string CoreMath::VectorToString(const Vector4 &v){
    string output = 
        "[" + 
        to_string(v.x) + ", " +
        to_string(v.y) + ", " +
        to_string(v.z) + ", " +
        to_string(v.w) + 
        "]" ;
    return output;
}


string CoreMath::VectorToString(const Vector3 &v){
    string output = 
        string("[") +
        string("x=") + to_string(v.x) + ", " +
        string("y=") + to_string(v.y) + 
        string("z=") + to_string(v.z) + 
        "]" ;
    return output;
}


string CoreMath::VectorToString(const Vector2 &v){
    string output = 
        string("[") +
        string("x=") + to_string(v.x) + ", " +
        string("y=") + to_string(v.y) + 
        "]" ;
    return output;
}


string CoreMath::MatrixToString(const Matrix &A) {
    string output = 
        "----------\n" +
        to_string(A.m11) + " " + to_string(A.m12) + " " + to_string(A.m13) + " " + to_string(A.m14) + "\n" +
        to_string(A.m21) + " " + to_string(A.m22) + " " + to_string(A.m23) + " " + to_string(A.m24) + "\n" +
        to_string(A.m31) + " " + to_string(A.m32) + " " + to_string(A.m33) + " " + to_string(A.m34) + "\n" +
        to_string(A.m41) + " " + to_string(A.m42) + " " + to_string(A.m43) + " " + to_string(A.m44) + "\n" +
        "----------";
    return output;
}


void CoreMath::PrintMatrix(const Matrix &A) {
    std::string output = 
        "----------\n" +
        to_string(A.m11) + " " + to_string(A.m12) + " " + to_string(A.m13) + " " + to_string(A.m14) + "\n" +
        to_string(A.m21) + " " + to_string(A.m22) + " " + to_string(A.m23) + " " + to_string(A.m24) + "\n" +
        to_string(A.m31) + " " + to_string(A.m32) + " " + to_string(A.m33) + " " + to_string(A.m34) + "\n" +
        to_string(A.m41) + " " + to_string(A.m42) + " " + to_string(A.m43) + " " + to_string(A.m44) + "\n" +
        "----------";
    Logger(output);
}
