//
// Created by zack on 11/11/21.
//

#include "GameObjectManager.h"

GameObject GameObjectManager::listener;

std::unordered_map<std::basic_string<char>, std::any> GameObject::globalParams;
GameObjectManager manager; // TODO temp until somebetter to make sure its inited atleast one for testing and normal runtime

GameObjectManager::GameObjectManager()
{
    std::any value = 512.0f;
    std::any preprocess = 0.0f;
    std::any lerpHRIR = 0.0f;
    std::any phaseAlign = 1.0f;
    std::any crossFade = 1.0f;
    GameObject::SetParam(std::string("Overlap"), value);
    GameObject::SetParam("Preprocess", preprocess);
    GameObject::SetParam("LerpHRIR", lerpHRIR);
    GameObject::SetParam("PhaseAlign", phaseAlign);
    GameObject::SetParam("CrossFade", crossFade);
}

uint64_t GameObjectManager::AddObject(uint64_t id)
{
    GameObject object;
    auto result = gameObjects.try_emplace(id, object);
    if(result.second)
        return id;
    return 0;
}

uint64_t GameObjectManager::RemoveObject(uint64_t  id)
{
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
    auto objToGet = gameObjects.find(id);
    if(objToGet != gameObjects.end())
    {
        obj = objToGet->second;
        return 1;
    }
    return -1;
}

void GameObjectManager::SetGameObjectTransform(uint64_t id, const Transform& transform)
{
    auto obj = gameObjects.find(id);
    if(obj == gameObjects.end())
        return;

    obj->second.SetTransform(transform);
}

void GameObjectManager::SetGameObjectPosition(uint64_t id, const IVector3& position)
{
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