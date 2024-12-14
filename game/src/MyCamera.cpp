#include "MyCamera.h"
#include "meta/MyCamera.generated.h"

SERIALIZE(MyCamera)
DESERIALIZE(MyCamera)
USER_OBJECT_FACTORY(MyCamera, Camera)


void MyCamera::Start(Node2D *self) {
    MyCamera *myCam = (MyCamera *) self;
    // Debug::Logger("MyCam->velocity", myCam->velocity);
    // Debug::Logger("MyCam->count", myCam->count);
}

 
void MyCamera::Update(Node2D *self) {
    MyCamera *myCam = (MyCamera *) self;
    if(CoreInput::IsKeyPressed(CoreInput::KeyCode::KEY_W)) {
        myCam->parent.transform.pos.y += myCam->velocity;
        myCam->count++;
        // Debug::Logger("up", myCam->parent.transform.pos.y);
    }
    else if(CoreInput::IsKeyPressed(CoreInput::KeyCode::KEY_S)) {
        myCam->parent.transform.pos.y -= myCam->velocity;
        myCam->count++;
        // Debug::Logger("down", myCam->parent.transform.pos.y);
    }
    else if(CoreInput::IsKeyPressed(CoreInput::KeyCode::KEY_A)) {
        myCam->parent.transform.pos.x -= myCam->velocity;
        myCam->count++;
        // Debug::Logger("left", myCam->parent.transform.pos.x);
    }
    else if(CoreInput::IsKeyPressed(CoreInput::KeyCode::KEY_D)) {
        myCam->parent.transform.pos.x += myCam->velocity;
        myCam->count++;
        // Debug::Logger("right", myCam->parent.transform.pos.x);
    }
}


void MyCamera::Shutdown(Node2D *self) {
    // Game logic..
}





// For record

/* void MyCamera::Serialize(Node2D *self) {
    Debug::Logger("Serialize");

    int n = ARRAYSIZE(MyCamera_fields);
    for(int i = 0; i < n; i++) {
        ReflectedStruct rs = MyCamera_fields[i];
        MetaField field;
        switch(field.type) {
            case meta_int : 
            {
                field.name = rs.name;
                field.type = meta_int;
                int *iP = (int *)( ((std::byte *) self) + rs.offset);
                field.value_i = *iP;
            } break;
            case meta_float : 
            {
                field.name = rs.name;
                field.type = meta_float;
                float *fP = (float *)( ((std::byte *) self) + rs.offset);
                field.value_f = *fP;
            } break;
            case meta_char_p : 
            {
            }break;
            default : break;
        }
        self->meta.push_back(field);
    }
} */

/* void MyCamera::DeSerialize(Node2D *self) { 
    MyCamera *o = (MyCamera *) self;
    auto it = self->meta.begin();
    auto end = self->meta.end();
    int n = ARRAYSIZE(MyCamera_fields);
    while(it != end) {
        MetaField current = *it;
        for(int i = 0; i < n; i++) {
            ReflectedStruct rs = MyCamera_fields[i];
            if(strcmp(rs.name, current.name) == 0) {
                std::byte *ptr = ((std::byte *) o) + rs.offset;
                switch(current.type) {
                    case meta_int : 
                    {
                        int *iPtr = (int *) ptr;
                        *iPtr = current.value_i;
                    } break;
                    case meta_float : 
                    {
                        float *fPtr = (float *) ptr;
                        *fPtr = current.value_f;
                    } break;
                    case meta_char_p : 
                    {

                    } break;
                }
                break;
            }
        }
        it++;
    }
} */
