#include <core/CoreGlobals.h>
#include <core/SceneGraph.h>
#include <core/Physics.fwd.h>
#include <platform/Graphics.h>
#include <stdexcept>
#include <utils/RUID.h>
#include <stack>
#include <set>
#include <unordered_map>
#include <utils/Debug.h>

using namespace SceneGraph;
std::unordered_map<std::string, SceneGraph::Scene*> scenes;
std::unordered_map<std::string, SceneGraph::Scene*> _scenes;


void SceneGraph::Init() {
}


std::string SceneGraph::GenerateSceneID(){
    return RUID::GenerateResourceUID(Signature::SCENE);
}


Node2D* SceneGraph::GetSceneRoot(Scene **scene) {
    return (*scene)->sceneRoot;
}


bool SceneGraph::AttachTo(Node2D *parent, Node2D *child){
    (parent)->children.push_back(child);
    child->parent = parent;
    return true;
}


std::vector<Node2D*> SceneGraph::GetSceneNodesByName(Scene **scene, std::string name) {
    std::vector<Node2D*> found;
    std::stack<Node2D*> stack;
    std::set<Node2D*> visited;
    stack.push((*scene)->sceneRoot);
    while(!stack.empty()){
        Node2D* current = stack.top();
        stack.pop();
        auto it = visited.find(current);
        if(it == visited.end()){
            visited.insert(current);
            if(current->name == name) {
                found.push_back(current);
            }
            stack.push(current);
            for(auto it = current->children.begin(); it != current->children.end(); ++it){
                Node2D* child = (Node2D*) *it;
                stack.push(child);
            }
        }
    }
    return found;
}


Scene* SceneGraph::CreateScene(std::string name, Camera *activeCamera, Node2D* root, std::string id) {
    try{
        if(_scenes.count(name) > 0) {
            throw std::invalid_argument("scene name already taken");
        }
        Scene *newScene = new Scene;
        newScene->id = id.empty() ? GenerateSceneID() : id;
        newScene->name = name;
        newScene->sceneRoot = root;
        newScene->activeCamera = activeCamera;
        if(root == nullptr) {
            Empty *newEmpty = GameObject::CreateEmptyObject(
                name + "-root", "Empty",
                Vector2{0.0f, 0.0f},
                Vector2{1.0f, 1.0f},
                0.0f
                ); 
            newScene->sceneRoot = reinterpret_cast<Node2D*>(newEmpty);
        }
        scenes[newScene->id] = newScene;
        _scenes[newScene->name] = newScene;
        // CoreGlobals::gameObjectNameToID[newScene->name] = newScene->id;
        return newScene;
    }catch(const std::exception &e) {
        Debug::Logger("Fail Create Scene", e.what());
        return nullptr;
    }
};


Scene* SceneGraph::GetSceneByName(std::string name) {
    if(_scenes.count(name) == 0) {
        Debug::Logger("SceneGraph:: ","Scene name not found", name); 
        return nullptr;
    }
    return _scenes.at(name);
}


void SceneGraph::InitPass(Scene *scene) {
    Node2D* root = scene->sceneRoot;
    std::stack<Node2D *> stack;
    std::set<Node2D *> visited;
    stack.push(root);

    while(!stack.empty()){
        Node2D* current = stack.top();
        stack.pop();
        auto it = visited.find(current);
        if(it == visited.end()){
            if(current->behavior.DeSerialize) {
                current->behavior.DeSerialize(current);
            }

            if(current->behavior.Start) {
                current->behavior.Start(current);
            }
            visited.insert(current);
            for(auto it = current->children.begin(); it != current->children.end(); ++it){
                Node2D* child = (Node2D*) *it;
                stack.push(child);
            }
        }
    }
}


void SceneGraph::UpdatePass(Scene *scene, unsigned int fps, double deltaTime) {
    Node2D* root = scene->sceneRoot;
    CoreGeometry::BoundingRect *frustum = &(scene->activeCamera->geometry.AABB);
    std::stack<Node2D *> stack;
    std::set<Node2D *> visited;
    stack.push(root);

    while(!stack.empty()){
        Node2D* current = stack.top();
        stack.pop();
        auto it = visited.find(current);
        if(it == visited.end()){
            GameObject::Empty *renderable = reinterpret_cast<Empty*>(current);
            CoreMath::Matrix S = CoreMath::CreateScaleMatrix(renderable->transform.scale);
            CoreMath::Matrix R = CoreMath::CreateZRotationMatrix(renderable->transform.rotation);
            CoreMath::Matrix T = CoreMath::CreateTranslationMatrix(renderable->transform.pos.xy);
            // renderable->transform.Local = CoreMath::Multiply(CoreMath::Multiply(R, T), S); 
            renderable->transform.Local = CoreMath::Multiply(T, CoreMath::Multiply(R, S)); 

            if(renderable->attribute.parent){
                GameObject::Empty *parent = reinterpret_cast<Empty*>(current->parent);
                renderable->transform.World = CoreMath::Multiply(
                    parent->transform.World,
                    renderable->transform.Local
                    );
                renderable->transform.worldPos = CoreMath::VectorAdd(parent->transform.worldPos, renderable->transform.pos);
                Debug::Logger("World POS 1 = ", VectorToString(renderable->transform.worldPos));
                renderable->transform.worldPos = CoreMath::Multiply(T, renderable->transform.pos);
                Debug::Logger("World POS 2 = ", VectorToString(renderable->transform.worldPos));
            }else{
                renderable->transform.World = renderable->transform.Local;
                renderable->transform.worldPos = renderable->transform.pos;
            }

            switch(current->type){
                case GameObject::Type::SPRITE : 
                {
                    Sprite *sp = reinterpret_cast<Sprite*>(current);
                    CoreGeometry::UpdateAABB(
                        &sp->geometry.AABB, 
                        sp->geometry.vertices,
                        renderable->transform.World
                        );
                    CorePhysics::BoxCollider *spCollider = (CorePhysics::BoxCollider*) sp->collider;
                    spCollider->AABB = sp->geometry.AABB;
                    if(CoreGeometry::Intersect(frustum, &sp->geometry.AABB)) {
                        scene->drawable.push_back(current);
                    }
                    break;
                }
                case GameObject::Type::ANIMATED_SPRITE : 
                {
                    AnimatedSprite *as = reinterpret_cast<AnimatedSprite*>(current);
                    Sprite *sp = &as->sprite;
                    CoreGeometry::UpdateAABB(
                        &sp->geometry.AABB, 
                        sp->geometry.vertices,
                        renderable->transform.World
                        );
                    if(CoreGeometry::Intersect(frustum, &sp->geometry.AABB)) {
                        scene->drawable.push_back(current);
                    }
                    break;
                }
                case GameObject::Type::TEXT : 
                {
                    Text *text = reinterpret_cast<Text*>(current);
                    CoreGeometry::UpdateAABB(
                        &text->geometry.AABB, 
                        text->geometry.vertices,
                        renderable->transform.World
                        );
                    if(CoreGeometry::Intersect(frustum, &text->geometry.AABB)) {
                        scene->drawable.push_back(current);
                    }
                    break;
                }
                case GameObject::Type::CAMERA : 
                {
                    Camera *cm = reinterpret_cast<Camera*>(current);
                    Vector2 zoom = Vector2{cm->transform.pos.z, cm->transform.pos.z};
                    CoreGeometry::UpdateAABB(
                        &cm->geometry.AABB, 
                        cm->geometry.vertices,
                        CoreMath::Multiply(T, CoreMath::CreateScaleMatrix(zoom)) 
                        );
                    cm->up = CoreMath::Multiply(R, Vector4{0.0f, 1.0f, 0.0f, 0.0f});
                    cm->view = CoreMath::ViewSpaceMatrix(cm->transform.pos, cm->up);
                    if(scene->activeCamera == cm) {
                        Graphics::UpdateViewProjectionMatrix(cm);
                        scene->drawable.push_back(current);
                    }
                    break;
                }
                default : break;
            }

            if(current->behavior.Update) {
                current->behavior.Update(current, fps, deltaTime);
            }

            visited.insert(current);
            for(auto it = current->children.begin(); it != current->children.end(); ++it){
                Node2D* child = (Node2D*) *it;
                stack.push(child);
            }
        }
    }

    // SortSceneDrawable(scene->drawable);
    
}


// TODO: Shutdown pass should be called whenever scene changes or engine shutdown
void SceneGraph::ShutdownPass(Scene *scene) {
    Node2D* root = scene->sceneRoot;
    std::stack<Node2D *> stack;
    std::set<Node2D *> visited;
    stack.push(root);
    while(!stack.empty()){
        Node2D* current = stack.top();
        stack.pop();
        auto it = visited.find(current);
        if(it == visited.end()){
            if(current->behavior.Shutdown) {
                current->behavior.Shutdown(current);
            }
            visited.insert(current);
            for(auto it = current->children.begin(); it != current->children.end(); ++it){
                Node2D* child = (Node2D*) *it;
                stack.push(child);
            }
        }
    }
}


void SceneGraph::DrawPass(Scene *scene) {
    for(Node2D* node : scene->drawable) {
        switch(node->type){
            case GameObject::Type::SPRITE : 
            {
                Sprite *sp = reinterpret_cast<Sprite*>(node);
                Graphics::Draw(sp);
                break;
            }
            case GameObject::Type::ANIMATED_SPRITE:
            {
                AnimatedSprite *as = reinterpret_cast<AnimatedSprite*>(node);
                Graphics::Draw(as);
                break;
            }
            case GameObject::Type::TEXT : 
            {
                Text *text = reinterpret_cast<Text*>(node);
                Graphics::Draw(text);
                break;
            }
            case GameObject::Type::CAMERA : 
            {
                Camera *cam = reinterpret_cast<Camera*>(node);
                Graphics::Draw(cam);
                break;
            }
            default : break;
        }
    }
    scene->drawable.clear();
}


void SceneGraph::SortSceneDrawable(std::vector<Node2D*> &drawable) {
    if(drawable.size() <= 1) return;
    if(drawable.size() == 2) {
        if(drawable[0]->zIndex >= drawable[1]->zIndex) {
            Node2D *tmp = drawable[0];
            drawable[0] = drawable[1];
            drawable[1] = tmp;
        }
        return;
    }
    int middle = std::floor(drawable.size() / 2);
    std::vector<Node2D*> left = std::vector<Node2D*>(drawable.begin(), drawable.begin() + middle);
    std::vector<Node2D*> right = std::vector<Node2D*>(drawable.begin() + middle, drawable.end());

    SortSceneDrawable(left);
    SortSceneDrawable(right);

    int i, j, k;
    i = j  = k = 0;
    int n = left.size();
    int m = right.size();
    while ((i < n) && ((j < m))) {
        if(left[i]->zIndex >= right[j]->zIndex) {
            drawable[k] = right[j];
            j++;
        }else{
            drawable[k] = left[i];
            i++;
        }
        k++;
    }

    while(i < n) {
        drawable[k] = left[i];
        i++;
        k++;
    }

    while(j < n) {
        drawable[k] = right[j];
        j++;
        k++;
    }
}

