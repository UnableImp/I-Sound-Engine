//
// Created by zacke on 3/2/2022.
//

#ifndef I_SOUND_ENGINE_DESERIALIZEDDISTANCE_H
#define I_SOUND_ENGINE_DESERIALIZEDDISTANCE_H

#include "DeserializeFilter.h"

class DeserializedDistance : public DeserializedFilter
{
public:
    DeserializedDistance(rapidjson::Value& object);
    ErrorNum BuildFilter(Filter<float>** filter,  PackageManager& manager);
    ErrorNum BuildFilter(Filter<float>** filter,  PackageManager& manager, GameObject& obj);
};


#endif //I_SOUND_ENGINE_DESERIALIZEDDISTANCE_H
