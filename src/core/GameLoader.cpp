#include <core/GameLoader.h>
#include <core/CoreGlobals.h>
#include <platform/IO.h>
#include <api/Meta.h>
#include <string>
#include <utils/rapidjson/document.h>
#include <utils/rapidjson/prettywriter.h>
#include <utils/rapidjson/writer.h>
#include <utils/rapidjson/stringbuffer.h>
#include <utils/rapidjson/error/en.h>
#include <utils/Debug.h>
#include <any>
#include <stack>
#include <set>
#include <cassert>

using namespace CoreGlobals;

// rapidjson conflicted with wingdi,
// so we have to undef this macro
#undef GetObject


/*
 * Scene Related
 * */


bool GameLoader::LoadLevelFromFile(std::string filePath){
    IO::FileBuffer file = IO::OpenAndReadFile("./" + RESOURCE_BASE_PATH + "/" + filePath);
    if(file.buffer == nullptr) {
        return false;
    }

    rapidjson::Document DOM;
    DOM.SetObject();

    DOM.Parse(file.buffer);
    free(file.buffer);
    if(DOM.HasParseError()) {
        rapidjson::ParseErrorCode err = DOM.GetParseError();
        Debug::Logger("Error Parsing JSON : \n", rapidjson::GetParseError_En(err));
        return false;
    }

    rapidjson::Value &scene_name = DOM["scene_name"];
    Debug::Logger("Parsing json successful, document root is Object : ", scene_name.GetString());

    rapidjson::Value &nodes         = DOM["nodes"];
    rapidjson::Value &node_details  = DOM["node_details"];
    assert(nodes.Size() == node_details.Size());

    Debug::Logger("========== Loading Game Objects ===========");
    for(auto it = node_details.Begin(); it != node_details.End(); ++it) {
        if(!it->IsObject()) continue;
        rapidjson::Value &node      = it->GetObject();
        rapidjson::Value &transform = node["transform"].GetObject();
        rapidjson::Value &pos       = transform["pos"].GetArray();
        rapidjson::Value &scale     = transform["scale"].GetArray();
        CoreMath::Vector2 _pos      = CoreMath::CreateVector2(pos[0].GetFloat(), pos[1].GetFloat());
        CoreMath::Vector2 _scale    = CoreMath::CreateVector2(scale[0].GetFloat(), scale[1].GetFloat());
        float _rot                  = transform["rot"].GetFloat();

        std::string id   = node["id"].GetString();
        std::string name = node["name"].GetString();
        //TODO: Better rename tag to ClassName
        std::string tag  = node["tag"].IsString() ? node["tag"].GetString() : "";

        Node2D* current = nullptr;
        switch(node["type"].GetInt()){
            case GameObject::Type::SPRITE : 
            {
                GameResource::Material *material = GameResource::GetMaterialByID(node["material"].GetString());
                Node2D *sp = (Node2D*) GameObject::CreateSprite(
                    name, tag, material,
                    _pos, _scale, _rot,
                    id
                    );
                current = sp;
                if(CoreGlobals::gameTypesFactory.count(tag) > 0) {
                    auto factory = CoreGlobals::gameTypesFactory[tag];
                    current = factory(sp);
                };
                break;
            };
            case GameObject::Type::CAMERA : 
            {
                Node2D *cam = (Node2D*) GameObject::CreateCamera(name, tag, _pos, id);
                current = cam;
                if(CoreGlobals::gameTypesFactory.count(tag) > 0) {
                    auto factory = CoreGlobals::gameTypesFactory[tag];
                    current = factory(cam);
                };
                break;
            };
            case GameObject::Type::TEXT :
            {
                GameResource::Font *font = GameResource::GetFontByID(node["font"].GetString());
                Node2D *text = (Node2D*) GameObject::CreateText(node["text"].GetString(), name, font, _pos, node["size"].GetInt(), id);
                current = text;
                if(CoreGlobals::gameTypesFactory.count(tag) > 0) {
                    auto factory = CoreGlobals::gameTypesFactory[tag];
                    current = factory(text);
                };
                break;
            }
            case GameObject::Type::EMPTY : 
            {
                current = (Node2D*) GameObject::CreateEmptyObject(name, tag, _pos, _scale, _rot, id);
                break;
            };
            default : break;
        }

        // Register current to globals
        CoreGlobals::nodes[id] = current;
        CoreGlobals::_nodes[name].push_back(current);
        
        // parse object state
        rapidjson::Value &state = node["state"].GetArray();
        if(!state.Empty()) {
            std::vector<MetaField> metaFields;
            for(auto it = state.Begin(); it != state.End(); it++) {
                MetaField field;

                rapidjson::Value &member = it->GetObject();
                int type = member["type"].GetInt();
                field.type = static_cast<MetaType>(type);

                // char* tmp = new char[strlen(member["name"].GetString())+1];
                // strcpy(tmp, member["name"].GetString());
                // field.name = tmp;
                field.name = std::string(member["name"].GetString());
                Debug::Logger("Registering Meta Name", field.name, " for : ", name);

                if(member["value"].IsInt() && field.type == MetaType::meta_int) {
                    field.value_i = member["value"].GetInt();
                }
                else if(member["value"].IsFloat() && field.type == MetaType::meta_float){
                    field.value_f = member["value"].GetFloat();
                }
                else if(member["value"].IsArray()){
                }
                metaFields.push_back(field);
            }
            current->meta = metaFields;
        }
        // End loop
    }

    Debug::Logger("========== Connecting Game Objects ===========\n");
    for(auto it = nodes.Begin(); it != nodes.End(); ++it) {
        rapidjson::Value &node = it->GetObject();
        if(!node["parent"].IsNull()) {
            std::string currentId = node["id"].GetString();
            std::string parentId = node["parent"].GetString();
            if(CoreGlobals::nodes.count(currentId) == 0 || CoreGlobals::nodes.count(parentId) == 0) {
                Debug::Logger("child or parent are invalid, check your level file");
                return false;
            }
            GameObject::Node2D* current = CoreGlobals::nodes[currentId];
            GameObject::Node2D* parent = CoreGlobals::nodes[parentId];
            SceneGraph::AttachTo(parent, current);
            Debug::Logger("Node Connected : ", parent->name, " <-> ", current->name);
        }
    }

    Debug::Logger("========== Building Scene ===========\n");
    GameObject::Node2D *root = CoreGlobals::nodes[DOM["root"].GetString()];
    SceneGraph::Scene *s = SceneGraph::CreateScene(
        DOM["scene_name"].GetString(), 
        (Camera *) CoreGlobals::nodes[DOM["active_camera"].GetString()],
        // CoreGlobals::cameras[DOM["active_camera"].GetString()],
        CoreGlobals::nodes[DOM["root"].GetString()],
        DOM["scene_id"].GetString()
        );
    CoreGlobals::activeScene = s;
    Debug::Logger("Scene", s->name, "Is Loaded, Set as active");
    

    return true;
}


bool GameLoader::SaveLevelToFile(SceneGraph::Scene *scene, std::string dir, std::string fileName) {
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(buffer);
    rapidjson::Document DOM;
    DOM.SetObject();

    // Prep rapidjson docs
    rapidjson::MemoryPoolAllocator allocator = DOM.GetAllocator();
    rapidjson::Value nodes;
    rapidjson::Value nodes_details;
    nodes.SetArray();
    nodes_details.SetArray();

    // Populate nodes_details
    std::stack<GameObject::Node2D*> stack;
    std::set<GameObject::Node2D*> visited;
    stack.push(scene->sceneRoot);
    while(!stack.empty()) {
        GameObject::Node2D* current = stack.top();
        stack.pop();
        std::set<Node2D*>::iterator it= visited.find(current);
        if(it == visited.end()){
            Debug::Logger("visit : ", current->name);
            GameObject::Empty *e = reinterpret_cast<GameObject::Empty*>(current);

            // content of node
            rapidjson::Value node_link;
            node_link.SetObject();
            rapidjson::Value parent;
            parent.SetNull();
            if(current->parent) {
                parent.SetString(current->parent->id.c_str(), allocator);
            }
            rapidjson::Value aid(current->id.c_str(), allocator);
            node_link.AddMember("id", aid, allocator);
            node_link.AddMember("parent", parent, allocator);
            nodes.PushBack(node_link, allocator);

            // content of node_details
            rapidjson::Value node; node.SetObject();
            rapidjson::Value id(current->id.c_str(), allocator);
            rapidjson::Value name(current->name.c_str(), allocator);
            rapidjson::Value type; type.SetInt((int) current->type);
            rapidjson::Value zindex; zindex.SetInt(current->zIndex);
            rapidjson::Value tag;
            if(current->tag.empty()) {
                tag.SetNull();
            }else{
                tag.SetString(current->tag.c_str(), allocator);
            }

            node.AddMember("id", id, allocator);
            node.AddMember("name", name, allocator);
            node.AddMember("tag", tag, allocator);
            node.AddMember("type", type, allocator);
            node.AddMember("zindex", zindex, allocator);

            // node_details transform
            rapidjson::Value transform;
            rapidjson::Value pos;
            rapidjson::Value scale;
            rapidjson::Value rot;
            transform.SetObject();
            pos.SetArray();
            scale.SetArray();
            rot.SetFloat(e->transform.rotation);
            pos.PushBack(e->transform.pos.f[0], allocator);
            pos.PushBack(e->transform.pos.f[1], allocator);
            scale.PushBack(e->transform.scale.x, allocator);
            scale.PushBack(e->transform.scale.y, allocator);
            transform.AddMember("pos", pos, allocator);
            transform.AddMember("scale", scale, allocator);
            transform.AddMember("rot", rot, allocator);
            node.AddMember("transform", transform, allocator);
            
            switch(current->type){
                case GameObject::Type::SPRITE : 
                {
                    GameObject::Sprite *sp = reinterpret_cast<GameObject::Sprite*>(current);
                    rapidjson::Value mId(sp->material->id.c_str(), allocator);
                    node.AddMember("material", mId, allocator);
                    SaveGameResourceToFile(sp->material);
                    break;
                }
                case GameObject::Type::CAMERA : 
                {
                    GameObject::Camera *cm = reinterpret_cast<GameObject::Camera*>(current);
                    rapidjson::Value up;
                    up.SetArray();
                    up.PushBack(cm->up.f[0], allocator);
                    up.PushBack(cm->up.f[1], allocator);
                    up.PushBack(cm->up.f[2], allocator);
                    up.PushBack(cm->up.f[3], allocator);
                    node.AddMember("up", up, allocator);
                    break;
                }
                case GameObject::Type::TEXT : 
                {
                    GameObject::Text *text = reinterpret_cast<GameObject::Text*>(current);
                    rapidjson::Value width;
                    width.SetInt(text->width);
                    rapidjson::Value height;
                    height.SetInt(text->height);
                    rapidjson::Value size(text->size);// size.SetInt(text->size);
                    rapidjson::Value font(text->font->id.c_str(), allocator);
                    rapidjson::Value textContent(text->text.c_str(), allocator);
                    node.AddMember("width", width, allocator);
                    node.AddMember("height", height, allocator);
                    node.AddMember("size", size, allocator);
                    node.AddMember("font", font, allocator);
                    node.AddMember("text", textContent, allocator);
                    SaveGameResourceToFile(text->font);
                    break;
                }
                default : break;
            }

            // Meta Serialize
            rapidjson::Value state;
            state.SetArray();
            // Save each recent states
            if(current->behavior.Serialize) {
                current->behavior.Serialize(current);
            }
            for(auto &metaField : current->meta) {
                rapidjson::Value field;
                rapidjson::Value fieldType;
                rapidjson::Value fieldName;
                rapidjson::Value fieldValue;

                field.SetObject();
                fieldName.SetString(metaField.name.c_str(), (metaField.name.length()), allocator);
                fieldType.SetInt((int) metaField.type);
                field.AddMember("type", fieldType, allocator);
                field.AddMember("name", fieldName, allocator);

                switch (metaField.type) {
                    case meta_int : 
                    {
                        fieldValue.SetInt(metaField.value_i);
                    } break;
                    case meta_float : 
                    {
                        fieldValue.SetFloat(metaField.value_f);
                    } break;
                    default: break;
                }
                field.AddMember("value", fieldValue, allocator);
                state.PushBack(field, allocator);
            }
            node.AddMember("state", state, allocator);
            nodes_details.PushBack(node, allocator);
            
            visited.insert(current);
            for(auto &v : current->children){
                stack.push(v);
            }
        }
    // End of traversal
    }

    rapidjson::Value scene_id(scene->id.c_str(), allocator);
    rapidjson::Value scene_name(scene->name.c_str(), allocator);
    rapidjson::Value scene_root(scene->sceneRoot->id.c_str(), allocator);
    rapidjson::Value active_camera(scene->activeCamera->attribute.id.c_str(), allocator);
    DOM.AddMember("scene_id", scene_id, allocator);
    DOM.AddMember("scene_name", scene_name, allocator);
    DOM.AddMember("active_camera", active_camera, allocator);
    DOM.AddMember("root", scene_root, allocator);
    DOM.AddMember("nodes", nodes, allocator);
    DOM.AddMember("node_details", nodes_details, allocator);

    DOM.Accept(w);
    Debug::Logger("Saving DOM:\n", buffer.GetString());

    char* data = (char*) buffer.GetString();
    IO::FileBuffer file = { data, (unsigned long) buffer.GetSize()};

    if(fileName.empty()) {
        fileName = scene->name;
    }
    std::string filePath = "./" + RESOURCE_BASE_PATH + "/" + dir + "/" + fileName + SCENE_EXTENSION;
    Debug::Logger(filePath);
    IO::SaveFile(&file, filePath);

    return true;
}


/*
 * Game Resources Related
 * */


bool GameLoader::SaveGameResourceToFile(GameResource::Material *material, std::string fileName, std::string dir) {
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(buffer);
    rapidjson::Document DOM;
    DOM.SetObject();

    rapidjson::MemoryPoolAllocator<> allocator = DOM.GetAllocator();
    rapidjson::Value id(material->id.c_str(), allocator);
    rapidjson::Value name(material->name.c_str(), allocator);
    rapidjson::Value main_texture(material->mainTexture->id.c_str(), allocator);
    rapidjson::Value shader(material->shader->id.c_str(), allocator);
    rapidjson::Value shader_parameter;
    shader_parameter.SetObject();
    for(auto &sv : (material->shaderParameters)){
        switch (sv.second.dataType) {
            case GameResource::ShaderParamType::INTEGER : 
            {
                rapidjson::Value value;
                value.SetInt(std::any_cast<int>(sv.second.value));
                shader_parameter.AddMember(rapidjson::StringRef(sv.first.c_str()), value, allocator);
                break;
            }
            case GameResource::ShaderParamType::FLOATING : 
            {
                rapidjson::Value value;
                value.SetFloat(std::any_cast<float>(sv.second.value) );
                shader_parameter.AddMember(rapidjson::StringRef(sv.first.c_str()), value, allocator);
                break;
            }
            case GameResource::ShaderParamType::VEC4 : 
            {
                CoreMath::Vector4 vec = std::any_cast<CoreMath::Vector4>(sv.second.value);
                rapidjson::Value value;
                value.SetArray();
                for(int i = 0; i < 4; i++) {
                    value.PushBack(vec.f[i], allocator);
                }
                shader_parameter.AddMember(rapidjson::StringRef(sv.first.c_str()), value, allocator);
                break;
            }
            default : 
            {
                break;
            }
        }
    }

    DOM.AddMember("id", id, allocator);
    DOM.AddMember("name", name, allocator);
    DOM.AddMember("main_texture", main_texture, allocator);
    DOM.AddMember("shader", shader, allocator);
    DOM.AddMember("shader_parameter", shader_parameter, allocator);

    DOM.Accept(w);
    Debug::Logger("DOM", buffer.GetString());

    IO::FileBuffer file = {
        const_cast<char*>(buffer.GetString()),
        (unsigned long) buffer.GetSize()
    };

    if(fileName.empty()) {
       fileName = material->name;
    }

    fileName = "./" + RESOURCE_BASE_PATH + "/" + dir + "/" + fileName + MATERIAL_EXTENSION;

    return IO::SaveFile(&file, fileName);
}


bool GameLoader::SaveGameResourceToFile(GameResource::Shader *shader, std::string fileName, std::string dir) {
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(buffer);
    rapidjson::Document DOM;
    DOM.SetObject();

    rapidjson::MemoryPoolAllocator<> allocator = DOM.GetAllocator();
    rapidjson::Value id(shader->id.c_str(), allocator);
    rapidjson::Value name(shader->name.c_str(), allocator);
    rapidjson::Value file_path(shader->filePath.c_str(), allocator);

    DOM.AddMember("id", id, allocator);
    DOM.AddMember("name", name, allocator);
    DOM.AddMember("file_path", file_path, allocator);

    DOM.Accept(w);
    Debug::Logger("DOM", buffer.GetString());

    IO::FileBuffer file = {
        const_cast<char*>(buffer.GetString()),
        (unsigned long) buffer.GetSize()
    };

    if(fileName.empty()) {
        fileName = shader->name;
    }

    fileName = "./" + RESOURCE_BASE_PATH + "/" + dir + "/" + fileName + SHADER_EXTENSION;
    
    return IO::SaveFile(&file, fileName);
}


bool GameLoader::SaveGameResourceToFile(GameResource::Texture *texture, std::string fileName, std::string dir) {
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(buffer);
    rapidjson::Document DOM;
    DOM.SetObject();

    rapidjson::MemoryPoolAllocator<> allocator = DOM.GetAllocator();
    rapidjson::Value id(texture->id.c_str(), allocator);
    rapidjson::Value name(texture->name.c_str(), allocator);
    rapidjson::Value file_path(texture->filePath.c_str(), allocator);

    DOM.AddMember("id", id, allocator);
    DOM.AddMember("name", name, allocator);
    DOM.AddMember("file_path", file_path, allocator);

    DOM.Accept(w);
    Debug::Logger("DOM", buffer.GetString());

    IO::FileBuffer file = {
        const_cast<char*>(buffer.GetString()),
        (unsigned long) buffer.GetSize()
    };

    if(fileName.empty()) {
        fileName = texture->name;
    }

    fileName = "./" + RESOURCE_BASE_PATH + "/" + dir + "/" + fileName + TEXTURE_EXTENSION;

    return IO::SaveFile(&file, fileName);
}


bool GameLoader::SaveGameResourceToFile(GameResource::Font *font, std::string fileName, std::string dir) {
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(buffer);
    rapidjson::Document DOM;
    DOM.SetObject();

    rapidjson::MemoryPoolAllocator<> allocator = DOM.GetAllocator();
    rapidjson::Value id(font->id.c_str(), allocator);
    rapidjson::Value name(font->name.c_str(), allocator);
    rapidjson::Value file_path(font->filePath.c_str(), allocator);
    rapidjson::Value font_size; font_size.SetInt(font->fontResource->size);

    DOM.AddMember("id", id, allocator);
    DOM.AddMember("name", name, allocator);
    DOM.AddMember("file_path", file_path, allocator);
    DOM.AddMember("font_size", font_size, allocator);

    DOM.Accept(w);
    Debug::Logger("DOM", buffer.GetString());

    IO::FileBuffer file = {
        const_cast<char*>(buffer.GetString()),
        (unsigned long) buffer.GetSize()
    };

    if(fileName.empty()) {
        fileName = font->name;
    }

    fileName = "./" + RESOURCE_BASE_PATH + "/" + dir + "/" + fileName + FONT_EXTENSION;

    return IO::SaveFile(&file, fileName);

}


bool GameLoader::LoadGameResourcesFromDirectory(std::string dir) {
    Debug::Logger("Loading Game Resource From File :", dir);

    std::string base = "./" + RESOURCE_BASE_PATH + "/" + dir;

    Debug::Logger("\n=========== Loading Textures =============\n");
    std::string pattern = base + "/" + "*.texture.json";
    std::vector<std::string> files = IO::ListDirFiles(pattern);
    for (auto &fileName : files) {
        IO::FileBuffer file = IO::OpenAndReadFile(base + fileName);
        if(!file.buffer) {
            return false;
        }

        rapidjson::Document DOM;
        DOM.SetObject();
        DOM.Parse(file.buffer);

        if(DOM.HasParseError()) {
            rapidjson::ParseErrorCode err = DOM.GetParseError();
            Debug::Logger("Error Parsing JSON : \n", rapidjson::GetParseError_En(err));
            return false;
        }

        // Process
        std::string file_path = DOM["file_path"].GetString();
        Debug::Logger("texture filepath = ", file_path);
        Debug::Logger("texture name = ", DOM["name"].GetString());
        GameResource::CreateTexture(
            DOM["name"].GetString(), 
            file_path,
            DOM["id"].GetString()
            );

        free(file.buffer);
    }

    Debug::Logger("\n=========== Loading Shaders =============\n");
    pattern = base + "/" + "*.shader.json";
    files   = IO::ListDirFiles(pattern);
    for (auto &fileName : files) {
        IO::FileBuffer file = IO::OpenAndReadFile(base + fileName);

        rapidjson::Document DOM;
        DOM.SetObject();
        DOM.Parse(file.buffer);
        if(DOM.HasParseError()) {
            rapidjson::ParseErrorCode err = DOM.GetParseError();
            Debug::Logger("Error Parsing JSON : \n", rapidjson::GetParseError_En(err));
            return false;
        }

        // Process
        std::string file_path = DOM["file_path"].GetString();
        Debug::Logger("shader filepath = ", file_path);
        Debug::Logger("shader name = ", DOM["name"].GetString());
        GameResource::CreateShader(
            DOM["name"].GetString(), 
            file_path,
            DOM["id"].GetString()
            );

        free(file.buffer);
    }

    Debug::Logger("\n=========== Loading Materials =============\n");
    pattern = base + "/" + "*.material.json";
    files   = IO::ListDirFiles(pattern);
    for (auto &fileName : files) {
        IO::FileBuffer file = IO::OpenAndReadFile(base + fileName);

        rapidjson::Document DOM;
        DOM.SetObject();
        DOM.Parse(file.buffer);
        if(DOM.HasParseError()) {
            rapidjson::ParseErrorCode err = DOM.GetParseError();
            Debug::Logger("Error Parsing JSON : \n", rapidjson::GetParseError_En(err));
            return false;
        }

        // Process
        Debug::Logger("material name = ", DOM["name"].GetString());
        Debug::Logger("material texture = ", DOM["main_texture"].GetString());
        Debug::Logger("material shader = ", DOM["shader"].GetString());
        Debug::Logger("material params is object = ", DOM["shader_parameter"].IsObject() ? "yes" : "no");

        GameResource::ShaderParams materialParams;
        if(DOM["shader_parameter"].IsObject() && !DOM["shader_parameter"].IsNull()) {
            rapidjson::Value &sp = DOM["shader_parameter"].GetObject();
            for(auto it = sp.MemberBegin(); it != sp.MemberEnd(); ++it) {
                if(it->value.IsArray()){
                    rapidjson::Value &p = it->value.GetArray();
                    switch(p.Size()) {
                        case 4 : 
                        {
                            CoreMath::Vector4 vec4 = CoreMath::CreateVector4(p[0].GetFloat(), p[1].GetFloat(), p[2].GetFloat(), p[3].GetFloat());
                            materialParams[it->name.GetString()] = { GameResource::ShaderParamType::VEC4, vec4 };
                            Debug::Logger(it->name.GetString(), "(VEC4): ", CoreMath::VectorToString(vec4));
                        } break;
                        case 3 : 
                        {
                            CoreMath::Vector3 vec3 = CoreMath::CreateVector3(p[0].GetFloat(), p[1].GetFloat(), p[2].GetFloat());
                            materialParams[it->name.GetString()] = { GameResource::ShaderParamType::VEC3, vec3 };
                            Debug::Logger(it->name.GetString(), "(VEC3): ", CoreMath::VectorToString(vec3));
                        } break;
                    }
                }else if(it->value.IsFloat()){
                    Debug::Logger(it->name.GetString(), "(FLOATING): ", it->value.GetFloat());
                    materialParams[it->name.GetString()] = GameResource::ShaderParamData{
                        GameResource::ShaderParamType::FLOATING,
                        it->value.GetFloat()
                    };
                }else if(it->value.IsInt()) {
                    Debug::Logger(it->name.GetString(), "(INTEGER)", it->value.GetInt());
                    materialParams[it->name.GetString()] = GameResource::ShaderParamData{
                        GameResource::ShaderParamType::INTEGER,
                        it->value.GetInt()
                    };
                }
            }
            GameResource::CreateMaterial(
                DOM["name"].GetString(), 
                CoreGlobals::textures.at( DOM["main_texture"].GetString() ),
                CoreGlobals::shaders.at( DOM["shader"].GetString() ),
                &materialParams,
                DOM["id"].GetString()
                );
        } else {
            GameResource::CreateMaterial( 
                DOM["name"].GetString(), 
                CoreGlobals::textures.at( DOM["main_texture"].GetString() ),
                CoreGlobals::shaders.at( DOM["shader"].GetString() ),
                nullptr,
                DOM["id"].GetString()
                );
        }

        free(file.buffer);
    }

    Debug::Logger("\n=========== Loading Fonts =============\n");
    pattern = base + "/" + "*.font.json";
    files   = IO::ListDirFiles(pattern);
    for (auto &fileName : files) {
        IO::FileBuffer file = IO::OpenAndReadFile(base + fileName);

        rapidjson::Document DOM;
        DOM.SetObject();
        DOM.Parse(file.buffer);
        if(DOM.HasParseError()) {
            rapidjson::ParseErrorCode err = DOM.GetParseError();
            Debug::Logger("Error Parsing JSON : \n", rapidjson::GetParseError_En(err));
            return false;
        }

        // Process
        std::string file_path = DOM["file_path"].GetString();
        int font_size = DOM["font_size"].GetInt();
        Debug::Logger("font filepath = ", file_path);
        Debug::Logger("font name = ", DOM["name"].GetString());
        Debug::Logger("font size = ", font_size);
        GameResource::CreateFontResource(
            file_path,
            font_size,
            DOM["id"].GetString()
            );

        free(file.buffer);
    }

    // Finish
    return true;

}
