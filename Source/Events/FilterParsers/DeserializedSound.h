//
// Created by zack on 10/24/21.
//

#ifndef I_SOUND_ENGINE_DESERIALIZEDSOUND_H
#define I_SOUND_ENGINE_DESERIALIZEDSOUND_H

#include "DeserializeFilter.h"

class DeserializedSound : public DeserializedFilter
{
public:
    DeserializedSound(rapidjson::Value& object);
    virtual ErrorNum BuildFilter(Filter<float>** filter, std::unordered_map<uint64_t, SoundData>& table) override;

private:
    uint64_t playID;
    float volume;
    int loopCount;
    int shiftStart;
    int shiftUp;
    int shiftDown;
};


#endif //I_SOUND_ENGINE_DESERIALIZEDSOUND_H
