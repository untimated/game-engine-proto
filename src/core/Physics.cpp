#include <core/Physics.h>
#include <core/CoreGlobals.h>
#include <exception>
#include <utils/Debug.h>

using namespace CorePhysics;

CorePhysics::World* CoreGlobals::physicsWorld = nullptr;

double stepAccumulateTime = 0.0f;
double targetStepTime     = 1/60.0f;  // set this to follow engine target frame rate
double tolerance          = targetStepTime * 0.2;  // 20% fluctuation tolerance


bool CorePhysics::WorldMake() {
    try{
        CoreGlobals::physicsWorld = new CorePhysics::World();
        return true;
    }catch (std::exception &e){
        return false;
    }
}


bool CorePhysics::WorldDestroy() {
    try{
        for(auto &collider : CoreGlobals::physicsWorld->colliders) {
            delete collider;
        }
        return true;
    }catch (std::exception &e){
        return false;
    }
}


BoxCollider* CorePhysics::CreateBoxCollider(GameObject::Empty *emptyObject, CoreGeometry::BoundingRect &boundingRect) {
    BoxCollider *newCollider = new BoxCollider();
    newCollider->type = CorePhysics::ColliderType::BOX_COLLIDER;
    newCollider->visible = true;
    newCollider->transform = emptyObject->transform;
    newCollider->AABB = boundingRect;
    newCollider->owner = emptyObject;
    return newCollider;
}


void CorePhysics::RegisterCollider(Collider *collider) {
    CoreGlobals::physicsWorld->colliders.push_back(collider);
}


static bool CheckColliderIntersection(Collider *source, Collider *target) {
    bool intersect = false;
    if(source->type == BOX_COLLIDER && target->type == BOX_COLLIDER) {
        BoxCollider *boxA = reinterpret_cast<BoxCollider*>(source);
        BoxCollider *boxB = reinterpret_cast<BoxCollider*>(target);
        return CoreGeometry::Intersect(&boxA->AABB, &boxB->AABB);
    }
    //else if...
    return intersect;
}


static void CollisionResolve(Collider *source, Collider *target) {
    if(source->type == BOX_COLLIDER && target->type == BOX_COLLIDER) {
        BoxCollider *boxA = reinterpret_cast<BoxCollider*>(source);
        BoxCollider *boxB = reinterpret_cast<BoxCollider*>(target);
        boxA->velocity = CoreMath::VectorMul(boxA->velocity, -1.0f);
    }
}


static void Integrate(Collider *collider, float deltaTime) {
    if(collider->type == BOX_COLLIDER) {
        BoxCollider *box = reinterpret_cast<BoxCollider*>(collider);
        box->transform.pos.x += box->velocity.x;
        box->transform.pos.y += box->velocity.y;
        collider->owner->transform.pos.x += box->velocity.x;
        collider->owner->transform.pos.y += box->velocity.y;
    }
}


/*
 * Case When it's too fast
 * |----|------|-----| Actual  (1.6ms)
 * |----*---|--------| Physics (3.3ms) -> Catchup=0, Skip=2
 * Case When it's too slow
 * |-----------------| Actual  (3.3ms)
 * |--------|--------| Physics (1.6ms) -> Catchup=2, Skip=0
 * Case When it's on time
 * |--------|--------| Actual  (3.3ms)
 * |--------|--------| Physics (3.3ms) -> Catchup=0, Skip=0
 * Approach:
 *  - accumulate deltaTime each physics step
 *  - decrement accumulate -= fixedStepPeriod;
 *  - perform step until accumulate <= 0
 *  - too fast case won't need any treatment as it's handled by the win32 sleep()
 *
 * */
void CorePhysics::Step(double deltaTime) {
    stepAccumulateTime += deltaTime;

    // catchup
    int count=0;
    while(stepAccumulateTime >= targetStepTime ) {
        // Debug::Logger("physics catching up ", count++, stepAccumulateTime);
        uint32_t n = CoreGlobals::physicsWorld->colliders.size();
        for(uint32_t i = 0; i < n ; i++) {
            Collider *source = CoreGlobals::physicsWorld->colliders[i];
            for(uint32_t j = i + 1; j < n; j++) {
                Collider *target = CoreGlobals::physicsWorld->colliders[j];
                if(CheckColliderIntersection(source, target)) {
                    Debug::Logger("Collision Detected");
                    CollisionResolve(source, target);
                }
            }
        }
        for(uint32_t i = 0; i < n ; i++) {
            Collider *collider = CoreGlobals::physicsWorld->colliders[i];
            Integrate(collider, deltaTime);
        }
        stepAccumulateTime = std::min<double>(stepAccumulateTime - targetStepTime, 0.0f);
    }

}

