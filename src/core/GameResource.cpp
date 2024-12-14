#include <core/GameResource_impl.h>
#include <core/CoreGlobals.h>
#include <platform/Graphics.h>
#include <utils/Debug.h>
#include <utils/RUID.h>
#include <exception>

using namespace GameResource;

std::unordered_map<std::string, GameResource::Resource*> CoreGlobals::resources;
std::unordered_map<std::string, GameResource::Material*> CoreGlobals::materials;
std::unordered_map<std::string, GameResource::Material*> CoreGlobals::_materials;
std::unordered_map<std::string, GameResource::Texture*> CoreGlobals::textures; 
std::unordered_map<std::string, GameResource::Texture*> CoreGlobals::_textures;
std::unordered_map<std::string, GameResource::Shader*> CoreGlobals::shaders;
std::unordered_map<std::string, GameResource::Shader*> CoreGlobals::_shaders;
std::unordered_map<std::string, GameResource::Font*> CoreGlobals::fonts;
std::unordered_map<std::string, GameResource::Font*> CoreGlobals::_fonts;


/*
 * Game Resource internal 
 * */

static std::string GenerateResourceID(Signature signature) {
    std::string uid;
    do {
        uid = RUID::GenerateResourceUID(signature);
    } while(CoreGlobals::resources.count(uid) > 0);
    return uid;
}


/*
 * Game Resource expose functions
 * */


Texture* GameResource::CreateTexture(std::string name, std::string filePath, std::string id){
    if(name.empty()){
        Debug::Logger("GameResource:: Name should not be empty");
        return nullptr;
    }
    if(filePath.empty()){
        Debug::Logger("GameResource:: Filepath should not be empty");
        return nullptr;
    }

    Texture *newTexture = new Texture;
    newTexture->id = id.empty() ? GenerateResourceID(Signature::TEXTURE) : id;
    newTexture->filePath = filePath;
    newTexture->name = name;

    if(!Graphics::CreateTexture(newTexture)){
        Debug::Logger("GameResource:: Error while constructing texture with platform graphics");
        return nullptr;
    }

    CoreGlobals::resources[newTexture->id] = newTexture;
    CoreGlobals::textures[newTexture->id] = newTexture;
    CoreGlobals::_textures[newTexture->name] = newTexture;
    Debug::Logger("GameResource:: Texture Resource Created with ID : ", newTexture->id, "\n");
    return newTexture;
}


bool GameResource::FreeTextureResource(Texture **tex) {
    return Graphics::RemoveTexture(*tex);
}


Texture* GameResource::GetDefaultTexture(){
    return GameResource::GetTextureByName("Default-Texture");
}


Texture* GameResource::GetTextureByName(std::string name){
    Debug::Logger("GameResource:: looking for texture named ", name);
    if(!CoreGlobals::_textures.count(name)) {
        Debug::Logger("GameResource:: texture not found ", name); 
        return nullptr;
    }
    return CoreGlobals::_textures.at(name);
}


// Shaders


Shader* GameResource::CreateShader(std::string name, std::string filePath, std::string id) {
    if(name.empty()){
        Debug::Logger("GameResource:: Name should not be empty");
        return nullptr;
    }

    if(filePath.empty()){
        Debug::Logger("GameResource:: Filepath should not be empty");
        return nullptr;
    }

    try{
        Shader *newShader = new Shader;
        newShader->id = id.empty() ? GenerateResourceID(Signature::SHADER) : id;
        newShader->filePath = filePath;
        newShader->name = name;

        if(!Graphics::CreateShader(newShader)){
            Debug::Logger("GameResource:: Error while constructing shader with platform graphics");
            return nullptr;
        }
        
        CoreGlobals::resources[newShader->id] = newShader;
        CoreGlobals::shaders[newShader->id] = newShader;
        CoreGlobals::_shaders[newShader->name] = newShader;
        Debug::Logger("GameResource:: Shader Resource Created with ID : ", newShader->id, "\n");

        return newShader;
    }catch(const std::exception &e){
        Debug::Logger("Fail create shader", e.what());
        return nullptr;
    }

}


bool  GameResource::FreeShaderResource(Shader **shader) {
    return Graphics::RemoveShader(*shader);
}


Shader* GameResource::GetDefaultShader(){
    return GetShaderByName("Default-Shader");
}


Shader* GameResource::GetShaderByName(std::string name){
    Debug::Logger("GameResource:: looking for shader named ", name);
    if(!CoreGlobals::_shaders.count(name)) {
        Debug::Logger("GameResource:: shader not found ", name); 
        return nullptr;
    }
    return CoreGlobals::_shaders.at(name);
}


// Materials


Material* GameResource::CreateMaterial(
    std::string name,
    Texture *mainTexture,
    Shader *shader,
    const ShaderParams *shaderParameters,
    std::string id
) {
    if(name.empty()){
        Debug::Logger("GameResource:: Name should not be empty");
        return nullptr;
    }

    Material *newMaterial = new Material;
    newMaterial->id = id.empty() ? GenerateResourceID(Signature::MATERIAL) : id;
    newMaterial->name = name;

    if(!shader){
        newMaterial->shader = GetDefaultShader();
    }else{
        newMaterial->shader = shader;
    }

    if(!mainTexture){
        newMaterial->mainTexture = GetDefaultTexture();
    }else{
        newMaterial->mainTexture = mainTexture;
    }

    if(shaderParameters) {
        newMaterial->shaderParameters = *shaderParameters;
    }else{
        newMaterial->shaderParameters = shader->parameterMeta;
    }

    if(!Graphics::CreateMaterial(newMaterial)){
        Debug::Logger("GameResource:: Error while registering material in Graphics API");
        return nullptr;
    }

    CoreGlobals::resources[newMaterial->id] = newMaterial;
    CoreGlobals::materials[newMaterial->id] = newMaterial;
    CoreGlobals::_materials[newMaterial->name] = newMaterial;
    Debug::Logger("GameResource:: Material Resource Created with ID : ", newMaterial->id, "\n");
    return newMaterial;
}


bool GameResource::FreeMaterialResource(Material **mat) {
    //TODO: should we also delete the mat here or in engine core?
    return Graphics::RemoveMaterial(*mat);
}


Material* GameResource::GetDefaultMaterial(){
    return GetMaterialByName("Default-Material");
}


Material* GameResource::GetMaterialByID(std::string id) {
    return CoreGlobals::materials[id];
}


Material* GameResource::GetMaterialByName(std::string name){
    Debug::Logger("looking for material named ", name);
    if(!CoreGlobals::_materials.count(name)) {
        Debug::Logger("GameResource:: material not found ", name); 
        return nullptr;
    }
    return CoreGlobals::_materials.at(name);
}


ShaderParamData GameResource::GetMaterialParameter(Material **mat, std::string key) {
    ShaderParamData data = {ShaderParamType::INTEGER, 0};
    auto it = (*mat)->shaderParameters.find(key);
    if(it == (*mat)->shaderParameters.end()) {
        Debug::Logger("Parameter not found = ", key);
        return data;
    }
    data = {
        it->second.dataType,
        it->second.value
    };
    return data;
}


bool GameResource::SetMaterialParameter(Material **mat, std::string key, std::any value) {
    if((*mat)->shaderParameters.count(key) == 0) {
        Debug::Logger("parameters key not found = ", key.c_str());
        return false;
    }
    (*mat)->shaderParameters[key].value = value;
    return Graphics::UpdateMaterialParameters(mat);
}


/* FONT */


GameResource::Font* GameResource::CreateFontResource(std::string fontPath, uint32_t size, std::string id) {
    GameResource::Font *newFontResource = new Font();
    FontLoader::Font *font = FontLoader::LoadFont(fontPath.c_str(), size);
    newFontResource->fontResource = font;
    newFontResource->name = font->family;
    newFontResource->id = id.empty() ? GenerateResourceID(Signature::FONT) : id;
    newFontResource->filePath = fontPath;
    if(CoreGlobals::fonts.count(newFontResource->id) > 0) {
        FreeFontResource(CoreGlobals::fonts[newFontResource->id]);
        delete CoreGlobals::fonts[newFontResource->id];
    }
    CoreGlobals::resources[newFontResource->id] = newFontResource;
    CoreGlobals::fonts[newFontResource->id] = newFontResource;
    CoreGlobals::_fonts[newFontResource->name] = newFontResource;
    Debug::Logger("GameResource:: Font Resource Created with ID : ", newFontResource->id, "\n");
    return newFontResource;
}


bool GameResource::FreeFontResource(GameResource::Font *font) {
    FontLoader::FreeFont(font->fontResource);
    return true;
}


GameResource::Font* GameResource::GetFontByID(std::string id) {
    if(CoreGlobals::fonts.count(id) == 0) {
        Debug::Logger("GameResource:: Font not found ", id); 
        return nullptr;
    }
    return CoreGlobals::fonts[id];
}


GameResource::Font* GameResource::GetFontByName(std::string name) {
    if(CoreGlobals::_fonts.count(name) == 0) {
        Debug::Logger("GameResource:: Font not found ", name); 
        return nullptr;
    }
    return CoreGlobals::_fonts[name];
}


/* SpriteSheet */


// TODO: no need for dedicated resource, just use game object
// SpriteSheet* GameResource::CreateSpriteSheet(std::string name, std::string filePath, std::string id) {
//     GameResource::SpriteSheet *newSpriteSheet = new SpriteSheet();
//     newSpriteSheet->id = id.empty() ? GenerateResourceID(Signature::TEXTURE) : id;
//     newSpriteSheet->name = name;
//     newSpriteSheet->filePath = filePath;
//
//     if(!Graphics::CreateTexture(static_cast<Texture*>(newSpriteSheet))){
//         Debug::Logger("GameResource:: CreateSpriteSheet's Texture failed ", name); 
//         return nullptr;
//     }
//
//     if(CoreGlobals::spriteSheets.count(newSpriteSheet->id) > 0) {
//         FreeTextureResource((Texture **)CoreGlobals::spriteSheets[newSpriteSheet->id]);
//         delete CoreGlobals::spriteSheets[newSpriteSheet->id];
//     }
//     CoreGlobals::resources[newSpriteSheet->id] = newSpriteSheet;
//     CoreGlobals::spriteSheets[newSpriteSheet->id] = newSpriteSheet;
//     CoreGlobals::_spriteSheets[newSpriteSheet->name] = newSpriteSheet;
//     Debug::Logger("GameResource:: SpriteSheet Resource Created with ID : ", newSpriteSheet->id, "\n");
//     return newSpriteSheet;
// }

