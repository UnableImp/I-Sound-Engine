//
// Created by zack on 11/11/21.
//

#include "GameObject.h"

void GameObject::SetPosition(const IVector3 &pos)
{
    transform.postion = pos;
}

void GameObject::SetTransform(const Transform &trans)
{
    transform = trans;
}

void GameObject::SetUp(IVector3 const& up)
{
    transform.up = up;
}
void GameObject::SetForward(IVector3 const& forward)
{
    transform.forward = forward;
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