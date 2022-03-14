//
// Created by zacke on 11/17/2021.
//

#ifndef I_SOUND_ENGINE_DESERIALIZED3DSOUND_H
#define I_SOUND_ENGINE_DESERIALIZED3DSOUND_H

#include "DeserializeFilter.h"

class Deserialize3DSound : public DeserializedFilter
{
public:
    Deserialize3DSound(rapidjson::Value& object);
    ErrorNum BuildFilter(Filter<float>** filter,  PackageManager& manager) override;
    ErrorNum BuildFilter(Filter<float>** filter,  PackageManager& manager, GameObject& obj) override;

    virtual ~Deserialize3DSound() {};
};

#endif //I_SOUND_ENGINE_DESERIALIZED3DSOUND_H
