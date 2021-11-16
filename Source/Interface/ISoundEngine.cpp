//
// Created by zack on 10/25/21.
//

#include "ISoundEngine.h"

ISoundEngine::ISoundEngine() : packageManager(), eventManager(packageManager.GetSounds()), realTimeAudio(eventManager)
{

}

ErrorNum ISoundEngine::Init()
{
    realTimeAudio.Start();
    return ErrorNum::NoErrors;
}

ErrorNum ISoundEngine::Update()
{
    // TODO switch events to be buffer baised
    return  NoErrors;
}

ErrorNum ISoundEngine::Shutdown()
{
    realTimeAudio.Stop();
    return NoErrors;
}

ErrorNum ISoundEngine::LoadPackage(std::string path)
{
    return packageManager.LoadPack(path);
}

ErrorNum ISoundEngine::UnloadPackage(std::string path)
{
    return packageManager.UnloadPack(path);
}

ErrorNum ISoundEngine::LoadEvents(std::string path)
{
    eventManager.ParseEvents(path);
    return NoErrors;
}

ErrorNum ISoundEngine::UnloadEvents(std::string path)
{
    // TODO add way to unload events
    return NoErrors;
}

uint64_t ISoundEngine::PostEvent(std::string eventName)
{
    return eventManager.AddEvent(eventName);
}

uint64_t ISoundEngine::PostEvent(uint64_t id)
{
    return eventManager.AddEvent(id);
}

uint64_t ISoundEngine::AddObject(uint64_t id)
{
    return gameObjectManager.AddObject(id);
}

uint64_t ISoundEngine::RemoveObject(uint64_t id)
{
    return gameObjectManager.RemoveObject(id);
}

void ISoundEngine::SetTransform(uint64_t id, const Transform& transform)
{
    gameObjectManager.SetGameObjectTransform(id, transform);
}

void ISoundEngine::SetPosition(uint64_t id, const IVector3& position)
{
    gameObjectManager.SetGameObjectPosition(id, position);
}
