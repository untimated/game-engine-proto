#include <api/Engine.h>
#include <EngineCore.h>
#include <EnginePlatformAPI.h>

std::unordered_map<std::string, FactoryFunctionType> CoreGlobals::gameTypesFactory;


bool Engine::RegisterTypeFactory(std::string typeName, FactoryFunctionType factory) {
    int alreadyExists = CoreGlobals::gameTypesFactory.count(typeName);
    if(alreadyExists) {
        return false;
    }
    CoreGlobals::gameTypesFactory[typeName] = factory;
    return true;
}


void Engine::SetGameFPS(uint32_t fps) {
    EnginePlatformAPI::SetGameFPS(fps);
}


