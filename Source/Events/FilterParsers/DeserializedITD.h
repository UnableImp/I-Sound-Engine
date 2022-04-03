//
// Created by zacke on 2/5/2022.
//

#ifndef I_SOUND_ENGINE_DESERIALIZEDITD_H
#define I_SOUND_ENGINE_DESERIALIZEDITD_H

#include "DeserializeFilter.h"

class DeserializedITD : public DeserializedFilter
{
public:
    DeserializedITD(rapidjson::Value& object);
    ErrorNum BuildFilter(Filter<float>** filter,  PackageManager& manager);
    ErrorNum BuildFilter(Filter<float>** filter,  PackageManager& manager, GameObject& obj);
private:
    float dopplerStength;
};


#endif //I_SOUND_ENGINE_DESERIALIZEDITD_H
