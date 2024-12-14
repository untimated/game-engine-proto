#include <EngineCore.h>
#include <cstdint>

using namespace GameObject;
using namespace GameResource;

SceneGraph::Scene* CoreGlobals::activeScene = nullptr;
std::string CoreGlobals::PROJECT_BASE_PATH;

Text debugText;

bool EngineCore::Start(){
    if(!GameLoader::LoadGameResourcesFromDirectory()) {
        return false;
    };

    Debug::Logger("\n=============Game Resources loaded==========\n");
    
    if(!Game::Init()) {
        return false;
    }

    SceneGraph::InitPass(CoreGlobals::activeScene);
    Game::Start(CoreGlobals::activeScene);

    GameResource::Font* courier = GameResource::CreateFontResource("C:\\Windows\\Fonts\\consola.ttf", 36);
    DebugDraw::CreateText("", "log", DebugDraw::TextAlignment::ALIGN_LEFT, Vector2{10.0f, 10.0f});

    CorePhysics::WorldMake();

    Sprite *player = reinterpret_cast<Sprite*>(CoreGlobals::_nodes["Player"][0]);
    Sprite *box = reinterpret_cast<Sprite*>(CoreGlobals::_nodes["Box"][0]);
    if(player && box) {
        CorePhysics::BoxCollider *playerCollider = CorePhysics::CreateBoxCollider((GameObject::Empty*) player, player->geometry.AABB);
        CorePhysics::BoxCollider *boxCollider = CorePhysics::CreateBoxCollider((GameObject::Empty*) box, box->geometry.AABB);
        if(playerCollider && boxCollider) {
            player->collider = playerCollider;
            box->collider = boxCollider;
            CorePhysics::RegisterCollider(player->collider);
            CorePhysics::RegisterCollider(box->collider);
            Debug::Logger("Registering Collider to ", player->attribute.name);
            Debug::Logger("Registering Collider to ", box->attribute.name);
        }
    }

    
/*
    Texture *animatedSpriteTexture = GameResource::CreateTexture("Animated-Sprite-Texture", "./resources/assets/SpriteSheet.png");
    Shader *animatedSpriteShader = GameResource::CreateShader("Animated-Sprite-Shader", "./resources/shaders/animated-sprite.hlsl");
    ShaderParams materialParams = {
        {"brightness", {ShaderParamType::FLOATING, 1.0f}},
        {"count", {ShaderParamType::INTEGER, 5}},
    };
    Material *animatedSpriteMaterial = GameResource::CreateMaterial(
        "Animated-Sprite-Material", 
        animatedSpriteTexture, 
        animatedSpriteShader, 
        nullptr
        );
    GameObject::AnimatedSprite *anim = GameObject::CreateAnimatedSprite("animated", "", animatedSpriteMaterial, 0, 5, Vector2{200.0f, 200.0f}, Vector2{0.0f,0.0f}, Vector2{1.0f, 1.0f}, 0.0f);
    SceneGraph::AttachTo(CoreGlobals::activeScene->sceneRoot, reinterpret_cast<Node2D*>(anim));
*/

    return true;
}


void EngineCore::UpdateAndRender(uint32_t fps, double deltaTime){
    Game::Update(fps, deltaTime);
    SceneGraph::UpdatePass(CoreGlobals::activeScene, fps, deltaTime);
    CorePhysics::Step(deltaTime);
    SceneGraph::DrawPass(CoreGlobals::activeScene);
    DebugDraw::DrawPass();
}


// Should this be handled on corresponding file ?
void EngineCore::Shutdown() {
    Debug::Logger("EngineCore:: Shutting down...");

    SceneGraph::ShutdownPass(CoreGlobals::activeScene);
    DebugDraw::Shutdown();

    for(auto &pair : CoreGlobals::textures) {
        GameResource::FreeTextureResource(&pair.second);
        delete pair.second;
    }
    CoreGlobals::textures.clear();
    CoreGlobals::_textures.clear();
    Debug::Logger("EngineCore:: textures cleared");

    for(auto &pair : CoreGlobals::shaders) {
        GameResource::FreeShaderResource(&pair.second);
        delete pair.second;
    }
    CoreGlobals::shaders.clear();
    CoreGlobals::_shaders.clear();
    Debug::Logger("EngineCore:: shaders cleared");

    for(auto &pair : CoreGlobals::materials) {
        GameResource::FreeMaterialResource(&pair.second);
        delete pair.second;
    }
    CoreGlobals::materials.clear();
    CoreGlobals::_materials.clear();
    Debug::Logger("EngineCore:: materials cleared");

    for(auto &pair : CoreGlobals::fonts) {
        GameResource::FreeFontResource(pair.second);
        delete pair.second;
    }
    CoreGlobals::fonts.clear();
    CoreGlobals::_fonts.clear();
    Debug::Logger("EngineCore:: Font resource are cleared");

    for(auto &pair : CoreGlobals::nodes) {
        auto node = pair.second;
        if(node->type == GameObject::Type::SPRITE) {
            GameObject::FreeSpriteGeometryResource((GameObject::Sprite **) &pair.second);
        }
        delete pair.second;
    }
    CoreGlobals::nodes.clear();
    CoreGlobals::_nodes.clear();
    Debug::Logger("EngineCore:: object nodes are cleared");

    CorePhysics::WorldDestroy();
    // SceneGraph::Shutdown();
}


/*
 * Others
 * */


static bool InitDefaultTextureAndMaterial(){
    Debug::Logger("Initializing Engine Core ========================== \n");

    Texture *defaultTexture = GameResource::CreateTexture("Default-Texture", "./assets/checker.png");
    if(defaultTexture == nullptr) return false;

    GameLoader::SaveGameResourceToFile(defaultTexture, "default");

    Shader *defaultSpriteShader = GameResource::CreateShader("Default-Shader", "./src/common_shader/sprite.hlsl");
    if(defaultSpriteShader == nullptr) return false;

    // GameLoader::SaveGameResourceToFile(defaultSpriteShader, "sprite");

    ShaderParams materialParams = {
        {"brightness", {ShaderParamType::FLOATING, 1.0f}},
        {"count", {ShaderParamType::INTEGER, 5}},
    };

    Material *defaultSpriteMaterial = GameResource::CreateMaterial(
        "Default-Material", 
        defaultTexture, 
        defaultSpriteShader, 
        &materialParams
        );
    if(defaultSpriteMaterial == nullptr) return false;

    // GameLoader::SaveGameResourceToFile(defaultSpriteMaterial, "sprite");
    
    ShaderParams plainMaterialParams = {
        {"brightness", {ShaderParamType::FLOATING, 0.5f}},
        {"color", {ShaderParamType::VEC4, CreateVector4(1.0f, 0.5f, 0.0f, 1.0f)}}
    };
    Shader *plainShader = GameResource::CreateShader("Plain-Shader", "./src/common_shader/plain.hlsl");
    if(plainShader == nullptr) return false;

    // GameLoader::SaveGameResourceToFile(plainShader, "plain");

    Material *plainMaterial = GameResource::CreateMaterial(
        "Plain-Material",
        nullptr,
        plainShader,
        &plainMaterialParams
        );
    if(defaultSpriteMaterial == nullptr) return false;

    // GameLoader::SaveGameResourceToFile(plainMaterial, "plain");

    Debug::Logger("Complete ========================== \n");
    return true;
}


void EngineCore::SetGameBasePath(const char *path) {
    CoreGlobals::PROJECT_BASE_PATH = path;
}

