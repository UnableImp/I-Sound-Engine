//
// Created by zack on 11/11/21.
//

#include "GameObject.h"

#include <utility>




void GameObject::SetPosition(const IVector3 &pos)
{
    transform.postion = pos;
    SetParamLocal("Updated", true);
}

void GameObject::SetTransform(const Transform &trans)
{
    transform = trans;
    SetParamLocal("Updated", true);
}

void GameObject::SetUp(IVector3 const& up)
{
    transform.up = up;
    SetParamLocal("Updated", true);
}
void GameObject::SetForward(IVector3 const& forward)
{
    transform.forward = forward;
    SetParamLocal("Updated", true);
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

//void GameObject::SetParam(std::basic_string<char> id, std::any item)
//{
//    globalParams[id] = item;
//}
//
//const std::any& GameObject::GetParam(std::basic_string<char> id)
//{
//    return globalParams[id];
//}
