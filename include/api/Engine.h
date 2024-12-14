#ifndef ENGINE_H
#define ENGINE_H

#if OS==WIN
#define DLLEXPORT extern "C" __declspec(dllexport)
#define DLLIMPORT extern "C" __declspec(dllexport)
#endif

#define EXPOSE
#define REFLECT
#define SKIP

#define COMBINE(WORD_A, WORD_B) WORD_A ## WORD_B

#define SERIALIZE(Type) \
    void Type::Serialize(Node2D *self) { \
        Debug::Logger("Serialize"); \
        self->meta.clear(); \
        int n = ARRAYSIZE(COMBINE(Type, _fields)); \
        for(int i = 0; i < n; i++) { \
            ReflectedStruct rs = COMBINE(Type, _fields)[i]; \
            MetaField field; \
            switch(rs.type) { \
                case meta_int : { \
                    field.name = rs.name; \
                    field.type = meta_int; \
                    int *iP = (int *)( ((std::byte *) self) + rs.offset); \
                    field.value_i = *iP; \
                } break; \
                case meta_float : {  \
                    field.name = rs.name; \
                    field.type = meta_float; \
                    float *fP = (float *)( ((std::byte *) self) + rs.offset); \
                    field.value_f = *fP; \
                } break; \
                case meta_char_p :  \
                {}break; \
                default : break; \
            } \
            self->meta.push_back(field); \
        } \
    }


/* if(strcmp(rs.name, current.name) == 0) {*/
#define DESERIALIZE(Type) \
    void Type::DeSerialize(Node2D *self) { \
        Type *o = (Type *) self; \
        auto it = self->meta.begin(); \
        auto end = self->meta.end(); \
        int n = ARRAYSIZE(COMBINE(Type, _fields)); \
        while(it != end) { \
            MetaField current = *it; \
            for(int i = 0; i < n; i++) { \
                ReflectedStruct rs = COMBINE(Type, _fields)[i]; \
                if(rs.name == current.name) { \
                    std::byte *ptr = ((std::byte *) o) + rs.offset; \
                    switch(current.type) { \
                        case meta_int : \
                        { \
                            int *iPtr = (int *) ptr; \
                            *iPtr = current.value_i; \
                        } break; \
                        case meta_float : \
                        { \
                            float *fPtr = (float *) ptr; \
                            *fPtr = current.value_f; \
                        } break; \
                        case meta_char_p :  \
                        { \
                        } break; \
                    } \
                    break; \
                } \
            } \
            it++; \
        } \
    }


#define GAME_CODE_HEADER_DEFINTION(StructName) \
    const std::string TAG = #StructName; \
    void Start(Node2D *self); \
    void Update(Node2D *self, uint32_t fps, float deltaTime); \
    void Shutdown(Node2D *self); \
    void Serialize(Node2D *self); \
    void DeSerialize(Node2D *self); \
    Node2D* Factory(Node2D *node); 


#define USER_OBJECT_FACTORY(Type, BaseType) \
    Node2D* Type::Factory(Node2D* node) { \
        BaseType *parent = (BaseType*) node; \
        Type *newObj = new Type(); \
        newObj->parent = *parent; \
        newObj->parent.attribute.behavior.Start = Start; \
        newObj->parent.attribute.behavior.Update = Update; \
        newObj->parent.attribute.behavior.Serialize = Serialize; \
        newObj->parent.attribute.behavior.DeSerialize = DeSerialize; \
        newObj->parent.attribute.behavior.Shutdown = Shutdown; \
        return (Node2D*) newObj; \
    }


#include <core/CoreGlobals.h>
#include <core/GameObject.h>
#include <core/GameResource.h>
#include <core/Math.h>
#include <core/Math_impl.h>
#include <core/SceneGraph.h>
#include <core/GameLoader.h>
#include <platform/Input.h>
#include <platform/Input_impl.h>
#include <utils/Debug.h>


namespace Engine {
    bool RegisterTypeFactory(std::string typeName, FactoryFunctionType factory);
    void SetGameFPS(uint32_t fps);
};



#endif
