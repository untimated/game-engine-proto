#ifndef GAMERESOURCE_IMPL_H
#define GAMERESOURCE_IMPL_H

#include <core/GameResource.h> 

using namespace GameResource;


namespace GameResource {

    Texture* CreateTexture(std::string name, std::string filePath, std::string id = "");
    bool FreeTextureResource(Texture **tex);
    Texture* GetDefaultTexture();
    Texture* GetTextureByName(std::string name);

    Shader* CreateShader(std::string name, std::string filePath, std::string id = "");
    bool FreeShaderResource(Shader **shader);
    Shader* GetDefaultShader();
    Shader* GetShaderByName(std::string name);

    Material* CreateMaterial(
        std::string name,
        Texture *mainTexture,
        Shader *shader,
        const ShaderParams *shaderParameters,
        std::string id = ""
    );
    bool FreeMaterialResource(Material **mat);
    Material* GetDefaultMaterial();
    Material* GetMaterialByID(std::string id);
    Material* GetMaterialByName(std::string name);
    ShaderParamData GetMaterialParameter(Material **mat, std::string key);
    bool SetMaterialParameter(Material **mat, std::string key, std::any value);

    Font* CreateFontResource(std::string filePath, uint32_t size, std::string id = "");
    bool FreeFontResource(Font* font);
    Font* GetDefaultFont();
    Font* GetFontByID(std::string id);
    Font* GetFontByName(std::string name);

    // GameScript<GameObject::Node2D*>* GetScriptByName(std::string filename);
    // GameScript* GetScriptByName(std::string filename);
}

#endif
