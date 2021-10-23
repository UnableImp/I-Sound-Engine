//
// Created by zack on 10/22/21.
//

#include "DeserializedPCM.h"
#include "Filters/WavContainer.h"

DeserializedPCM::DeserializedPCM(rapidjson::Value &object)
{
    auto wavObject = object.GetObject();

    if(wavObject.HasMember("PlayID"))
        playID = wavObject["PlayID"].GetUint64();
    if(wavObject.HasMember("Volume"))
        volume = wavObject["Volume"].GetFloat();
}

ErrorNum DeserializedPCM::BuildFilter(Filter<float> **filter, std::unordered_map<uint64_t, SoundData> &table)
{
    if(table.find(playID) == table.end())
        return ErrorNum::SoundNotLoaded;

    *filter = new WavContainer<float>(table[playID]);
    return ErrorNum::NoErrors;
}