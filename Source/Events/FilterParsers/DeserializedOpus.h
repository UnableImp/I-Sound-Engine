//
// Created by zack on 10/22/21.
//

#ifndef I_SOUND_ENGINE_DESERIALIZEDOPUS_H
#define I_SOUND_ENGINE_DESERIALIZEDOPUS_H

#include "DeserializeFilter.h"

class DeserializedOpus : public DeserializedFilter
{
public:
    DeserializedOpus(rapidjson::Value& object);
    virtual ErrorNum BuildFilter(Filter<float>** filter, std::unordered_map<uint64_t, SoundData>& table) override;

private:
    uint64_t playID;
    float volume;
};


#endif //I_SOUND_ENGINE_DESERIALIZEDOPUS_H
