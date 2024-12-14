#include "MySprite.h"
#include "meta/MySprite.generated.h"

SERIALIZE(MySprite);
DESERIALIZE(MySprite)
USER_OBJECT_FACTORY(MySprite, Sprite);


void MySprite::Start(Node2D *self) {
    MySprite *mySprite = (MySprite *) self;
    Debug::Logger("MySprite->velocity", mySprite->velocity);
    Debug::Logger("MySprite->count", mySprite->count);
}

 
void MySprite::Update(Node2D *self) {
    // Game logic..
}


void MySprite::Shutdown(Node2D *self) {
    // Game logic..
}
