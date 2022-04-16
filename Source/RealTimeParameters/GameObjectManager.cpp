//
// Created by zack on 11/11/21.
//

#include "GameObjectManager.h"

GameObject GameObjectManager::listener;

std::unordered_map<std::basic_string<char>, std::any> GameObject::globalParams;
GameObjectManager manager; // TODO temp until somebetter to make sure its inited atleast one for testing and normal runtime

GameObjectManager::GameObjectManager()
{
    GameObject::SetParamStatic("HRIRSet", 1.0f);
    GameObject::SetParamStatic("Overlap", 512.0f);
    GameObject::SetParamStatic("Preprocess", 1.0f);
    GameObject::SetParamStatic("PhaseAlign", 1.0f);
    GameObject::SetParamStatic("CrossFade", 1.0f);
    GameObject::SetParamStatic("HeadRadius", 0.0875f);
    GameObject::SetParamStatic("DistanceScaler", 1.0f);
    GameObject::SetParamStatic("Telemetries", 0.0f);
    GameObject::SetParamStatic("DSPLoad", 0.0f);
    GameObject::SetParamStatic("HRTFLoad", 0.0f);
    GameObject::SetParamStatic("ITDLoad", 0.0f);
    GameObject::SetParamStatic("HRTFLoadTemp", 0.0f);
    GameObject::SetParamStatic("ITDLoadTemp", 0.0f);
    GameObject::SetParamStatic("Woodworth", 1.0f);
    GameObject::SetParamStatic("UseHRTF", 1.0f);
    GameObject::SetParamStatic("UseITD" , 1.0f);
    GameObject::SetParamStatic("UseDistanceAtten", 1.0f);
    GameObject::SetParamStatic("Q", 0.7f);
    GameObject::SetParamStatic("LowpassType", 0.0f);
    GameObject::SetParamStatic("MaxSoundDistance", 100.0f);
    GameObject::SetParamStatic("RolloffFunc", 0.0f);
    GameObject::SetParamStatic("UseLowpass", 1.0f);
    GameObject::SetParamStatic("DistanceIntensity", 1.0f);
    GameObject::SetParamStatic("Volume", 1.0f);
    GameObject::SetParamStatic("Updated", 1.0f);
}

uint64_t GameObjectManager::AddObject(uint64_t id)
{
    std::lock_guard lock(m);
    GameObject object;
    auto result = gameObjects.try_emplace(id, object);
    if(result.second)
        return id;
    return 0;
}

uint64_t GameObjectManager::RemoveObject(uint64_t  id)
{
    std::lock_guard lock(m);
    auto obj = gameObjects.find(id);
    if (obj != gameObjects.end())
    {
        gameObjects.erase(obj);
        return id;
    }
    return 0;
}

int GameObjectManager::GetGameObject(uint64_t id, GameObject& obj) const
{
    std::lock_guard lock(m);
    auto objToGet = gameObjects.find(id);
    if(objToGet != gameObjects.end())
    {
        obj = objToGet->second;
        return 1;
    }
    return -1;
}

GameObject& GameObjectManager::operator[](uint64_t id)
{
    std::lock_guard lock(m);
    return gameObjects[id];
}

void GameObjectManager::SetGameObjectTransform(uint64_t id, const Transform& transform)
{
    std::lock_guard lock(m);
    auto obj = gameObjects.find(id);
    if(obj == gameObjects.end())
        return;

    obj->second.SetTransform(transform);
}

void GameObjectManager::SetGameObjectPosition(uint64_t id, const IVector3& position)
{
    std::lock_guard lock(m);
    auto obj = gameObjects.find(id);
    if(obj == gameObjects.end())
        return;
    obj->second.SetPosition(position);
}

void GameObjectManager::SetListenerTransform(const Transform& transform)
{
    listener.SetTransform(transform);
}

void GameObjectManager::SetListenerPosition(const IVector3& position)
{
    listener.SetPosition(position);
}

const Transform& GameObjectManager::GetListenerPosition()
{
    return listener.GetTransform();
}