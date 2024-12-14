#ifndef META_H
#define META_H

#include <core/Math.h>
#include <string>

enum MetaType {
    meta_int, //0
    meta_char_p, //1
    meta_float //2
};

struct ReflectedStruct {
    MetaType type = meta_int;
    std::string name;
    size_t offset;
};

struct MetaField {
    MetaType type = meta_int;
    std::string name;
    union {
        int value_i;
        float value_f;
        char* value_c;
        CoreMath::Vector4 value_v4;
    };
};

#endif
