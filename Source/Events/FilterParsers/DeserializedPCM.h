//
// Created by zack on 10/22/21.
//

#ifndef I_SOUND_ENGINE_DESERIALIZEDPCM_H
#define I_SOUND_ENGINE_DESERIALIZEDPCM_H

#include "DeserializeFilter.h"

class DeserializedPCM : public DeserializedFilter
{
public:
    DeserializedPCM(rapidjson::Value& object);
    virtual ErrorNum BuildFilter(Filter<float>** filter, std::unordered_map<uint64_t, SoundData>& table) override;

private:
    uint64_t playID;
    float volume;
};


#endif //I_SOUND_ENGINE_DESERIALIZEDPCM_H
