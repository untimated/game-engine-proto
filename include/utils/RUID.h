#ifndef RID_H
#define RID_H

#include <string>

/*
 * Header:  RUID.h
 * Impl:    RUID.cpp
 * Purpose: Generate Random UID from all types of resources
 *          such as scenes, sprites, materials, etc
 *          format = (Signature = 1)(Alphabet = 4)(Numeric = 6)
 * Author:  Michael Herman
 * */

enum Signature {
    SCENE,
    MATERIAL,
    TEXTURE,
    CAMERA,
    SHADER,
    FONT,
};

static unsigned int LinearCongruentialGenerator(
    const unsigned int modulus,
    const unsigned int multiplier,
    const unsigned int increment
);

static wchar_t GetResourceSignature(Signature signature);

namespace RUID {
    wchar_t* GenerateResourceUIDW(Signature signature); // TODO: deprecated
    std::string GenerateResourceUID(Signature signature, unsigned int seedNoise = 0);
}

#endif
