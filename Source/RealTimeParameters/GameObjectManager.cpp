//
// Created by zack on 11/11/21.
//

#include "GameObjectManager.h"

GameObjectManager::GameObjectManager()
{

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

uint64_t GameObjectManager::EventIDToGameObjectId(uint64_t id) const
{
    auto obj = eventsToGameObjets.find(id);
    if(obj != eventsToGameObjets.end())
        return obj->second;
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

void GameObjectManager::AddEventIDMaping(uint64_t gameObjId, uint64_t eventId)
{
    eventsToGameObjets.try_emplace(eventId, gameObjId);
}

void GameObjectManager::RemoveEventIDMaping(uint64_t eventId)
{
    auto obj = eventsToGameObjets.find(eventId);
    if(obj != eventsToGameObjets.end())
        eventsToGameObjets.erase(obj);
}