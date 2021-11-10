//
// Created by zack on 10/24/21.
//

#include "DeserializedSound.h"
#include "Filters/WavContainer.h"
#include "Filters/OpusContainer.h"

DeserializedSound::DeserializedSound(rapidjson::Value &object) : playID(-1), volume(0), loopCount(0), shiftStart(0), shiftDown(0), shiftUp(0)
{
    auto wavObject = object.GetObject();

    if(wavObject.HasMember("PlayID"))
        playID = wavObject["PlayID"].GetUint64();
    if(wavObject.HasMember("Volume"))
        volume = wavObject["Volume"].GetFloat();
    if(wavObject.HasMember("LoopCount"))
        loopCount = wavObject["LoopCount"].GetInt();
    if(wavObject.HasMember("PitchStart"))
        shiftStart = wavObject["PitchStart"].GetInt();
    if(wavObject.HasMember("ShiftCeil"))
        shiftUp = wavObject["ShiftCeil"].GetInt();
    if(wavObject.HasMember("ShiftFloor"))
        shiftDown = wavObject["ShiftFloor"].GetInt();

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

    auto* newSound = dynamic_cast<SoundContainer<float>*>(*filter);

    if(newSound)
    {
        newSound->SetLoopCount(loopCount);
        newSound->SetPitch(shiftStart);
        newSound->SetRandomPitchRange(shiftUp, shiftDown);
        newSound->SetVolume(volume);
    }

    return ErrorNum::NoErrors;
}