//
// Created by zack on 11/11/21.
//

#ifndef I_SOUND_ENGINE_GAMEOBJECT_H
#define I_SOUND_ENGINE_GAMEOBJECT_H

#include "Transform.h"

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

private:



};


#endif //I_SOUND_ENGINE_GAMEOBJECT_H
