//
// Created by zack on 11/11/21.
//

#include "GameObject.h"

void GameObject::SetPosition(const IVector3 &pos)
{

}

void GameObject::SetTransform(const Transform &trans)
{

}

void SetUp(IVector3 const& up)
{

}
void SetForward(IVector3 const& forward)
{

}

const Transform& GameObject::GetTransform() const
{
    return transform;
}

const IVector3& GameObject::GetPosition() const
{
    return transform.postion;
}

const IVector3& GameObject::GetUp() const
{
    return transform.up;
}

const IVector3& GameObject::GetForward() const
{
    return transform.forward;
}