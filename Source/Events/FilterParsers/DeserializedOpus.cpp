//
// Created by zack on 10/22/21.
//

#include "DeserializedOpus.h"
#include "Filters/OpusContainer.h"

DeserializedOpus::DeserializedOpus(rapidjson::Value &object)
{
    auto wavObject = object.GetObject();

    if(wavObject.HasMember("PlayID"))
        playID = wavObject["PlayID"].GetUint64();
    if(wavObject.HasMember("Volume"))
        volume = wavObject["Volume"].GetFloat();
}

ErrorNum DeserializedOpus::BuildFilter(Filter<float> **filter, std::unordered_map<uint64_t, SoundData> &table)
{
    if(table.find(playID) == table.end())
        return ErrorNum::SoundNotLoaded;

    *filter = new OpusContainer<float>(table[playID]);
    return ErrorNum::NoErrors;
}