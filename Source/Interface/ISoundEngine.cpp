//
// Created by zack on 10/25/21.
//

#include "ISoundEngine.h"

PackageManager* packageManager;
GameObjectManager* gameObjectManager;
EventManager* eventManager;
RealTimeAudio* realTimeAudio;

ISE_API ErrorNum ISoundEngine::StartUp()
{
    packageManager = new PackageManager();
    gameObjectManager = new GameObjectManager();
    eventManager = new EventManager(packageManager->GetSounds(), *gameObjectManager);
    realTimeAudio = new RealTimeAudio(*eventManager);
    return ErrorNum::NoErrors;
}

ISE_API ErrorNum ISoundEngine::Init()
{
    realTimeAudio->Start();
    return ErrorNum::NoErrors;
}

ISE_API ErrorNum ISoundEngine::Update()
{
    // TODO switch events to be buffer baised
    return  NoErrors;
}

ISE_API ErrorNum ISoundEngine::Shutdown()
{
    realTimeAudio->Stop();
    return NoErrors;
}

ISE_API ErrorNum ISoundEngine::LoadPackage(std::string path)
{
    return packageManager->LoadPack(path);
}

ISE_API ErrorNum ISoundEngine::UnloadPackage(std::string path)
{
    return packageManager->UnloadPack(path);
}

ISE_API ErrorNum ISoundEngine::LoadEvents(std::string path)
{
    eventManager->ParseEvents(path);
    return NoErrors;
}

ISE_API ErrorNum ISoundEngine::UnloadEvents(std::string path)
{
    // TODO add way to unload events
    return NoErrors;
}

ISE_API uint64_t ISoundEngine::PostEventString(std::string eventName)
{
    return eventManager->AddEvent(eventName);
}

ISE_API uint64_t ISoundEngine::PostEvent(uint64_t id)
{
    return eventManager->AddEvent(id);
}

ISE_API uint64_t ISoundEngine::AddObject(uint64_t id)
{
    return gameObjectManager->AddObject(id);
}

ISE_API uint64_t ISoundEngine::RemoveObject(uint64_t id)
{
    return gameObjectManager->RemoveObject(id);
}

ISE_API void ISoundEngine::SetTransform(uint64_t id, const Transform& transform)
{
    gameObjectManager->SetGameObjectTransform(id, transform);
}

ISE_API void ISoundEngine::SetPosition(uint64_t id, const IVector3& position)
{
    gameObjectManager->SetGameObjectPosition(id, position);
}

ISE_API void ISoundEngine::SetListenerTransform(const Transform& transform)
{
    GameObjectManager::SetListenerTransform(transform);
}

ISE_API void ISoundEngine::SetListernerPosition(const IVector3& position)
{
    GameObjectManager::SetListenerPosition(position);
}
