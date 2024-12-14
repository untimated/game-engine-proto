#include <core/CoreGlobals.h>
#include <core/GameObject_impl.h>
#include <core/GameResource_impl.h>
#include <cstdint>
#include <platform/Graphics.h>
#include <string>
#include <utils/Debug.h>
#include <utils/RUID.h>

using namespace GameObject;

std::unordered_map<std::string, GameObject::Node2D*> CoreGlobals::nodes;
std::unordered_map<std::string, std::vector<GameObject::Node2D*>> CoreGlobals::_nodes;
unsigned long CoreGlobals::nodeLastId = 100;
unsigned long CoreGlobals::emptyLastId = 100;
unsigned long CoreGlobals::spriteLastId = 100;
unsigned long CoreGlobals::cameraLastId = 100;
unsigned long CoreGlobals::textLastId = 100;


std::string GameObject::GenerateGameObjectID(Type type){
    std::string id;
    do {
        // Increment IDs until no duplicates were found
        switch(type) {
            case Type::SPRITE :
            {
                id = std::string("S" + std::to_string(CoreGlobals::spriteLastId++));
            }break;
            case Type::ANIMATED_SPRITE :
            {
                id = std::string("AS" + std::to_string(CoreGlobals::spriteLastId++));
            }break;
            case Type::CAMERA :
            {
                id = std::string("C" + std::to_string(CoreGlobals::cameraLastId++));
            }break;
            case Type::NODE2D :
            {
                id = std::string("N" + std::to_string(CoreGlobals::nodeLastId++));
            }break;
            case Type::EMPTY :
            {
                id =  std::string("E" + std::to_string(CoreGlobals::emptyLastId++));
            }break;
            case Type::TEXT :
            {
                id =  std::string("T" + std::to_string(CoreGlobals::textLastId++));
            }break;
            default :
            {
                id = std::string("S" + std::to_string(CoreGlobals::spriteLastId++));
            }
        };
    } while(CoreGlobals::nodes.count(id) >= 1);
    return id;
}


Node2D* GameObject::CreateNode2D(std::string name) {
    Node2D *newNode2D = new Node2D;
    newNode2D->id = GenerateGameObjectID(Type::NODE2D);
    newNode2D->name = name;
    newNode2D->type = Type::NODE2D;
    CoreGlobals::nodes[newNode2D->id] = newNode2D;
    CoreGlobals::_nodes[newNode2D->name].push_back((Node2D*) newNode2D);
    Debug::Logger("GameObject:: success creating object with id : ", newNode2D->id, "\n");
    return newNode2D;
}


Empty* GameObject::CreateEmptyObject(
    std::string name,
    std::string tag,
    CoreMath::Vector2 position,
    CoreMath::Vector2 scale,
    float rotation,
    std::string id
) {
    Empty *newEmpty = new Empty;
    newEmpty->attribute.id = id.empty() ? GenerateGameObjectID(Type::EMPTY) : id;
    newEmpty->attribute.name = name;
    newEmpty->attribute.type = Type::EMPTY;
    newEmpty->transform.pos = Vector4{position.x, position.y, 0.0f, 1.0f};
    newEmpty->transform.scale = scale;
    newEmpty->transform.rotation = rotation;
    newEmpty->transform.World = CoreMath::IdentityMatrix();
    newEmpty->transform.Local = CoreMath::IdentityMatrix();
    newEmpty->attribute.parent = nullptr;
    if(id.empty()) {
        CoreGlobals::nodes[newEmpty->attribute.id] = (Node2D*) newEmpty;
        CoreGlobals::_nodes[newEmpty->attribute.name].push_back((Node2D*) newEmpty);
    }
    Debug::Logger("GameObject:: success creating object with id : ", newEmpty->attribute.id, "\n");
    return newEmpty;
}


// Sprite


Sprite* GameObject::CreateSprite(
    std::string name,
    std::string tag,
    GameResource::Material *material,
    Vector2 position,
    Vector2 scale,
    float rotation,
    std::string id
){
    Sprite *newSprite = new Sprite();
    newSprite->attribute.id = id.empty() ? GenerateGameObjectID(Type::SPRITE) : id;
    newSprite->attribute.name = name;
    newSprite->attribute.tag = tag;
    newSprite->attribute.type = Type::SPRITE;
    newSprite->transform.pos = Vector4{position.x, position.y, 0.0f, 1.0f};
    newSprite->transform.rotation = rotation;
    newSprite->transform.scale = scale;
    newSprite->transform.World = CoreMath::IdentityMatrix();
    newSprite->transform.Local = CoreMath::IdentityMatrix();

    if(material == nullptr){
        newSprite->material = GameResource::GetDefaultMaterial();
    }else{
        newSprite->material = material;
    }

    GameResource::Texture *spriteTexture = newSprite->material->mainTexture;
    float texW = spriteTexture->dimension.x / 2.0f;
    float texH = spriteTexture->dimension.y / 2.0f;
    newSprite->geometry.vertices = {
        {-texW, texH, 0.0f, 1.0f},
        {texW, texH, 0.0f, 1.0f},
        {texW, -texH, 0.0f, 1.0f},
        {-texW, -texH, 0.0f, 1.0f},
    };

    newSprite->geometry.AABB = CoreGeometry::CreateAABB(newSprite->geometry.vertices);

    if(!Graphics::CreateGeometry(newSprite)) {
        Debug::Logger("GameObject:: Fail register sprite with name : ", name);
        return nullptr;
    }
    if(id.empty()) {
        // Only register when id is empty, because creation is handled on GameLoader (ln 104)
        // 'id' signaling that this object has id defined in level file
        CoreGlobals::nodes[newSprite->attribute.id] = (Node2D*) newSprite;
        CoreGlobals::_nodes[newSprite->attribute.name].push_back((Node2D*) newSprite);
    }
    Debug::Logger("GameObject:: success creating object with id : ", newSprite->attribute.id, "\n");
    return newSprite;
}

bool GameObject::FreeSpriteGeometryResource(Sprite **sp) {
    return Graphics::RemoveGeometry(*sp);
}


// Animated Sprite


AnimatedSprite* GameObject::CreateAnimatedSprite(
    std::string name,
    std::string tag,
    GameResource::Material *material,
    unsigned int startFrame,
    unsigned int fps,
    Vector2 frameDimension,
    Vector2 position,
    Vector2 scale,
    float rotation,
    std::string id
){
    AnimatedSprite *newAnimatedSprite = new AnimatedSprite();
    newAnimatedSprite->sprite.attribute.id = id.empty() ? GenerateGameObjectID(Type::ANIMATED_SPRITE) : id;
    newAnimatedSprite->sprite.attribute.name = name;
    newAnimatedSprite->sprite.attribute.tag = tag;
    newAnimatedSprite->sprite.attribute.type = Type::ANIMATED_SPRITE;
    newAnimatedSprite->sprite.transform.pos = Vector4{position.x, position.y, 0.0f, 1.0f};
    newAnimatedSprite->sprite.transform.rotation = rotation;
    newAnimatedSprite->sprite.transform.scale = scale;
    newAnimatedSprite->sprite.transform.World = CoreMath::IdentityMatrix();
    newAnimatedSprite->sprite.transform.Local = CoreMath::IdentityMatrix();
    newAnimatedSprite->frameDimension = frameDimension;
    newAnimatedSprite->currentFrame = startFrame;
    newAnimatedSprite->fps = fps;

    if(material == nullptr){
        Debug::Logger("GameObject:: Material required for animated sprite: ", name);
        return nullptr;
    }else{
        newAnimatedSprite->sprite.material = material;
    }

    GameResource::Texture *spriteTexture = newAnimatedSprite->sprite.material->mainTexture;
    float texW = spriteTexture->dimension.x;
    float texH = spriteTexture->dimension.y;
    float hW = frameDimension.x / 2.0f;
    float hH = frameDimension.y / 2.0f;
    newAnimatedSprite->sprite.geometry.vertices = {
        {-hW, hH, 0.0f, 1.0f },
        {hW, hH, 0.0f, 1.0f },
        {hW, -hH, 0.0f, 1.0f },
        {-hW, -hH, 0.0f, 1.0f},
    };
    uint32_t row = spriteTexture->dimension.x / frameDimension.x;
    uint32_t col = spriteTexture->dimension.y / frameDimension.y;
    newAnimatedSprite->totalFrames = row * col;
    newAnimatedSprite->frameDimensionNormalized.x = frameDimension.x / texW;
    newAnimatedSprite->frameDimensionNormalized.y = frameDimension.y / texH;
    newAnimatedSprite->pitch = texW / frameDimension.x;

    newAnimatedSprite->sprite.geometry.AABB = CoreGeometry::CreateAABB(newAnimatedSprite->sprite.geometry.vertices);

    if(!Graphics::CreateGeometry(&newAnimatedSprite->sprite)) {
        Debug::Logger("GameObject:: Fail register sprite with name : ", name);
        return nullptr;
    }
    if(id.empty()) {
        // Only register when id is empty, because creation is handled on GameLoader (ln 104)
        // 'id' signaling that this object has id defined in level file
        CoreGlobals::nodes[newAnimatedSprite->sprite.attribute.id] = (Node2D*) newAnimatedSprite;
        CoreGlobals::_nodes[newAnimatedSprite->sprite.attribute.name].push_back((Node2D*) newAnimatedSprite);
    }
    Debug::Logger("GameObject:: success creating object with id : ", newAnimatedSprite->sprite.attribute.id, "\n");
    return newAnimatedSprite;
}


// Camera


Camera* GameObject::CreateCamera(std::string name, std::string tag, const Vector2 pos, std::string id){
    Camera *newCamera = new Camera();
    newCamera->attribute.id = id.empty() ? GenerateGameObjectID(Type::CAMERA) : id;
    newCamera->attribute.name = name;
    newCamera->attribute.tag = tag;
    newCamera->attribute.type = Type::CAMERA;
    newCamera->transform.pos = Vector4{ pos.x, pos.y, 1.0f, 1.0f };
    newCamera->transform.scale = Vector2{1.0f, 1.0f};
    Vector2 screenDim = Graphics::GetScreenDimension(); 
    float hw = screenDim.x / 2.2f;
    float hh = screenDim.y / 2.2f;
    newCamera->geometry.AABB.bound = {-hw, -hh, hw, hh};
    newCamera->geometry.vertices = {
        {-hw, hh, 0.0f, 1.0f},
        {hw, hh, 0.0f, 1.0f},
        {hw, -hh, 0.0f, 1.0f},
        {-hw, -hh, 0.0f, 1.0f},
    };
    newCamera->geometry.showBoundingRect = true;
    newCamera->view = CoreMath::ViewSpaceMatrix(newCamera->transform.pos, newCamera->up);
    if(id.empty()) {
        CoreGlobals::nodes[newCamera->attribute.id] = (Node2D*) newCamera;
        CoreGlobals::_nodes[newCamera->attribute.name].push_back((Node2D*) newCamera);
    }
    Debug::Logger("GameObject:: success creating object with id : ", newCamera->attribute.id, "\n");
    return newCamera;
}


// Text

Text* GameObject::CreateText(std::string text, std::string name, GameResource::Font *font, const Vector2 pos, uint32_t size, std::string id) {
    Text *newText = new Text();
    newText->attribute.id = id.empty() ? GenerateGameObjectID(Type::TEXT) : id;
    newText->attribute.name = name;
    newText->attribute.type = Type::TEXT;
    newText->font = font;
    // newText->attribute.tag = "null";
    newText->transform.World = CoreMath::IdentityMatrix();
    newText->transform.Local = CoreMath::IdentityMatrix();
    newText->text = text;

    FontLoader::RenderText(
        &newText->surfaceBuffer,
        font->fontResource,
        text.c_str(),
        newText->width,
        newText->height
        );

    float scaleFactor = (float) size / font->fontResource->size;
    newText->transform.scale = Vector2{scaleFactor, scaleFactor};
    newText->transform.pos = Vector4{pos.x, pos.y, 0.0f, 1.0f};
    newText->transform.rotation = 0.0f;
    newText->size = size;

    float hw = newText->width / 2.0f;
    float hh = newText->height / 2.0f;
    newText->geometry.vertices = {
        {-hw, hh, 0.0f, 1.0f},
        {hw, hh, 0.0f, 1.0f},
        {hw, -hh, 0.0f, 1.0f},
        {-hw, -hh, 0.0f, 1.0f},
    };
    newText->geometry.AABB = CoreGeometry::CreateAABB(newText->geometry.vertices);
    if(!Graphics::CreateGeometry(newText)){
        Debug::Logger("GameObject:: fail creating Text geometry with id : ", newText->attribute.id, "\n");
    }
    if(id.empty()) {
        CoreGlobals::nodes[newText->attribute.id] = (Node2D*) newText;
        CoreGlobals::_nodes[newText->attribute.name].push_back((Node2D*) newText);
    }
    Debug::Logger("GameObject:: success creating Text object with id : ", newText->attribute.id, "\n");
    return newText;
}
