//
// Created by zack on 10/24/21.
//

#include "DeserializedSound.h"
#include "Filters/WavContainer.h"
#include "Filters/OpusContainer.h"

DeserializedSound::DeserializedSound(rapidjson::Value &object)
{
    auto wavObject = object.GetObject();

    if(wavObject.HasMember("PlayID"))
        playID = wavObject["PlayID"].GetUint64();
    if(wavObject.HasMember("Volume"))
        volume = wavObject["Volume"].GetFloat();
}

ErrorNum DeserializedSound::BuildFilter(Filter<float> **filter, std::unordered_map<uint64_t, SoundData> &table)
{
    auto sound = table.find(playID);
    if(sound == table.end())
        return ErrorNum::SoundNotLoaded;

    if(sound->second.audioType == Encoding::Opus)
        *filter = new OpusContainer<float>(sound->second);
    else
        *filter = new WavContainer<float>(sound->second);

    return ErrorNum::NoErrors;
}