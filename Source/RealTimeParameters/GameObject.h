//
// Created by zack on 11/11/21.
//

#ifndef I_SOUND_ENGINE_GAMEOBJECT_H
#define I_SOUND_ENGINE_GAMEOBJECT_H

#include "Transform.h"
#include <unordered_map>
#include <any>

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

    static void SetParam(std::basic_string<char> id, std::any item);
    static const std::any& GetParam(std::basic_string<char> id);

private:

    Transform transform;
    static std::unordered_map<std::basic_string<char>, std::any> globalParams;

};


#endif //I_SOUND_ENGINE_GAMEOBJECT_H
