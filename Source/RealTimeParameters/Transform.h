//
// Created by zack on 11/11/21.
//

#ifndef I_SOUND_ENGINE_TRANSFORM_H
#define I_SOUND_ENGINE_TRANSFORM_H

#include "IVector3.h"

struct Transform
{
    IVector3 postion = {0,0,0};
    IVector3 up = {0,1,0};
    IVector3 forward = {1,0,0};
};

#endif //I_SOUND_ENGINE_TRANSFORM_H
