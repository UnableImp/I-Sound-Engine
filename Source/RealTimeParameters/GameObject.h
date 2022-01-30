//
// Created by zack on 11/11/21.
//

#ifndef I_SOUND_ENGINE_GAMEOBJECT_H
#define I_SOUND_ENGINE_GAMEOBJECT_H

#include "Transform.h"
#include <unordered_map>
#include <any>
#include <string>

class GameObject
{
public:
    void SetTransform(Transform const& trans);
    void SetPosition(IVector3 const& pos);
    void SetUp(IVector3 const& up);
    void SetForward(IVector3 const& forward);

    const Transform& GetTransform() const;
    const IVector3& GetPosition() const;
    const IVector3& GetUp() const;
    const IVector3& GetForward() const;

    template<typename T>
    inline static void SetParam(std::basic_string<char> id, T item)
    {
        globalParams[id] = item;
    }

    template<typename T>
    inline static const T GetParam(std::basic_string<char> id)
    {
        const auto result = globalParams.find(id);
        if(result != globalParams.end())
            return std::any_cast<T>(result->second);
        assert(!"Failed to find key");
        return T();
    }

private:

    Transform transform;
    static std::unordered_map<std::basic_string<char>, std::any> globalParams;

};


#endif //I_SOUND_ENGINE_GAMEOBJECT_H
