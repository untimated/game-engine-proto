#include <api/Game.h>
#include <EngineCore.h>
#include <platform/Input_impl.h>
#include "GameGlobals.h"
#include "MyCamera.h"
#include "MySprite.h"

using namespace GameGlobals;

SceneGraph::Scene *GameGlobals::scene;

bool savingInProgress = false;


void Game::Init() {
    EngineCore::RegisterTypeFactory("MyCamera", MyCamera::Factory);
    EngineCore::RegisterTypeFactory("MySprite", MySprite::Factory);
    Debug::Logger("Game types factory registered");
}


void Game::Start(SceneGraph::Scene *activeScene) {
    scene = activeScene;
    Debug::Logger("Game Started");
}


void Game::Update() {
    if(CoreInput::IsKeyPressed(CoreInput::KeyCode::MOUSE_LEFT) && !savingInProgress) {
        Debug::Logger("Serialize..");
        savingInProgress = true;
        GameLoader::SaveLevelToFile(scene, "./game");
    }
    if(CoreInput::IsKeyPressed(CoreInput::KeyCode::MOUSE_RIGHT) && savingInProgress) {
        savingInProgress = false;
    }
}


void Game::Shutdown() {
}

