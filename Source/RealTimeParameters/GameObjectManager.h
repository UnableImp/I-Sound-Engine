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

    int GetGameObject(uint64_t id, GameObject& obj) const;

    void SetGameObjectTransform(uint64_t id, const Transform& transform);
    void SetGameObjectPosition(uint64_t id, const IVector3& position);

    static void SetListenerTransform(const Transform& transform);
    static void SetListenerPosition(const IVector3& position);

    static const Transform& GetListenerPosition();

private:
    std::unordered_map<uint64_t, GameObject> gameObjects;
    static GameObject listener;
};


#endif //I_SOUND_ENGINE_GAMEOBJECTMANAGER_H
