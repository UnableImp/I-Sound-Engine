//
// Created by zack on 11/11/21.
//

#ifndef I_SOUND_ENGINE_GAMEOBJECTMANAGER_H
#define I_SOUND_ENGINE_GAMEOBJECTMANAGER_H


#include <cstdint>
#include "GameObject.h"
#include "unordered_map"

class GameObjectManager
{
public:
    GameObjectManager();

    uint64_t AddObject(uint64_t id);
    uint64_t RemoveObject(uint64_t  id);

    uint64_t EventIDToGameObjectId(uint64_t id) const;
    int GetGameObject(uint64_t id, GameObject& obj) const;

    void SetGameObjectTransform(uint64_t id, const Transform& transform);
    void SetGameObjectPosition(uint64_t id, const IVector3& position);

    void AddEventIDMaping(uint64_t gameObjId, uint64_t eventId);
    void RemoveEventIDMaping(uint64_t eventId);

private:
    std::unordered_map<uint64_t, GameObject> gameObjects;
    std::unordered_map<uint64_t, uint64_t> eventsToGameObjets;
};


#endif //I_SOUND_ENGINE_GAMEOBJECTMANAGER_H
