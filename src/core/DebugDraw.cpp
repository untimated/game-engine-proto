#include <core/DebugDraw.h>
#include <core/GameResource_impl.h>
#include <platform/Graphics.h>
#include <platform/FontLoader.h>
#include <EnginePlatformAPI.h>
#include <utils/Debug.h>

using namespace DebugDraw;

std::vector<DebugObject*> DebugDraw::drawables;
std::unordered_map<std::string, DebugObject*> DebugDraw::debugObjects;
unsigned int lastID = 100;


static std::string GenerateID(Type type) {
    switch (type) {
        case Type::TEXT : 
        { 
            return std::string("T") + std::to_string(lastID);
        }
        break;
        default :
        { 
            return std::string("X") + std::to_string(lastID);
        }
        break;
    }
    lastID ++;
}


Text* DebugDraw::CreateText(std::string text, std::string name, TextAlignment alignment, const Vector2 margin) {
    Text* newText = new Text();
    newText->attribute.name = name;
    newText->attribute.id = GenerateID(Type::TEXT);

    GameResource::Font *font = GameResource::GetFontByName("Consolas");
    if(!font) {
        Debug::Logger("DebugDraw:: font might not exist");
        return nullptr;
    }
    
    newText->scaleFactor = (float) 12.0f / font->fontResource->size;
    newText->scale = Vector2{newText->scaleFactor, newText->scaleFactor};
    newText->rotation = 0.0f;
    
    long screenX, screenY;
    EnginePlatformAPI::GetWindowDimension(screenX, screenY);
    float upScale = 1.0 / newText->scaleFactor;
    newText->width = std::ceil(((float)screenX / 6) * upScale);
    newText->height = std::ceil(((float)screenY / 2) * upScale);

    FontLoader::RenderTextBox(
        &newText->surfaceBuffer,
        font->fontResource,
        text.c_str(),
        newText->width,
        newText->height
        );
    
    float textHalfWidth = ((float) newText->width  / 2) * newText->scaleFactor;
    float textHalfHeight = ((float) newText->height / 2) * newText->scaleFactor;
    switch(alignment) {
        case TextAlignment::ALIGN_RIGHT :
        {
            newText->pos = {
                ((float) screenX / 2) - textHalfWidth - margin.x,
                ((float) screenY / 2) - textHalfHeight - margin.y,
                0.0f, 1.0f
            };
        }
        break;
        case TextAlignment::ALIGN_LEFT :
        {
            newText->pos = {
                -((float) screenX / 2) + textHalfWidth + margin.x,
                ((float) screenY / 2) - textHalfHeight - margin.y,
                0.0f, 1.0f
            };
        }
        break;
        default : break;
    }
    newText->alignment = alignment;
    newText->margin = margin;

    if(!Graphics::CreateGeometry(newText)) {
        Debug::Logger("DebugDraw:: fail creating Text geometry with id : ", newText->attribute.id, "\n");
        return nullptr;
    }

    drawables.push_back((DebugObject*) newText);
    debugObjects[newText->attribute.name] = reinterpret_cast<DebugObject*>(newText);

    return newText;

}


void DebugDraw::SetText(std::string name, std::string text) {
    if(debugObjects.count(name) == 0) {
        Debug::Logger("DebugDraw:: cannot found object named : ", name);
        return;
    }

    // TODO: move this to upper scope?
    GameResource::Font *font = GameResource::GetFontByName("Consolas");
    if(!font) {
        Debug::Logger("DebugDraw:: font might not exist");
        return;
    }

    DebugDraw::Text *debugText = (Text*) debugObjects[name];
    delete debugText->surfaceBuffer;
    debugText->surfaceBuffer = nullptr;
    FontLoader::RenderTextBox(
        &debugText->surfaceBuffer,
        font->fontResource,
        text.c_str(),
        debugText->width,
        debugText->height
        );
    // FontLoader::PrintGlyphBuffer(debugText->surfaceBuffer, debugText->width, debugText->height);
    
    long screenX, screenY;
    EnginePlatformAPI::GetWindowDimension(screenX, screenY);
    float textHalfWidth = ((float) debugText->width  / 2) * debugText->scaleFactor;
    float textHalfHeight = ((float) debugText->height / 2) * debugText->scaleFactor;
    switch(debugText->alignment) {
        case TextAlignment::ALIGN_RIGHT :
        {
            debugText->pos = {
                ((float) screenX / 2) - textHalfWidth - debugText->margin.x,
                ((float) screenY / 2) - textHalfHeight - debugText->margin.y,
                0.0f, 1.0f
            };
        }
        break;
        case TextAlignment::ALIGN_LEFT :
        {
            debugText->pos = {
                -((float) screenX / 2) + textHalfWidth + debugText->margin.x,
                ((float) screenY / 2) - textHalfHeight - debugText->margin.y,
                0.0f, 1.0f
            };
        }
        break;
        default : break;
    }

    bool updated = Graphics::UpdateStaticTexture(
        debugText->textureResource,
        debugText->surfaceBuffer,
        debugText->width, debugText->height
        );
    if(!updated) {
        Debug::Logger("DebugDraw:: update text subresource failed");
        return;
    }
}


void DebugDraw::DrawPass() {
    for(auto drawable : drawables) {
        Type type = drawable->type;
        std::string name = drawable->name;
        switch(type) {
            case DebugDraw::Type::TEXT : 
            {
                Graphics::Draw((Text*) drawable);
            } break;
            default : break;
        }
    }
}


void DebugDraw::Shutdown() {
    Debug::Logger("Clearing debug drawables");
    for(auto drawable : drawables) {
        Type type = drawable->type;
        std::string name = drawable->name;
        switch(type) {
            case DebugDraw::Type::TEXT : 
            {
                Text *text = (Text*) drawable;
                Graphics::RemoveGeometry(text);
                delete text->surfaceBuffer;
            } break;
            default : break;
        }
    }
    drawables.clear();
}
