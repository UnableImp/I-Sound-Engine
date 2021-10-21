//
// Created by zack on 10/21/21.
//

#ifndef I_SOUND_ENGINE_REALTIMEAUDIOMODULE_H
#define I_SOUND_ENGINE_REALTIMEAUDIOMODULE_H

#include "RealTimeAudio/RealTimeAudio.h"
#include "gtest/gtest.h"
#include "benchmark/benchmark.h"
#include "Events/Event.h"
#include "Events/EventManager.h"

#include "AudioFormats/WavFile.h"
#include "AudioPackage/PackageEncoder.h"
#include "AudioPackage/PackageDecoder.h"
#include "Filters/WavContainer.h"
#include "Filters/OpusContainer.h"

#include <chrono>
#include <thread>

static void addFile(std::vector<WavFile>&)
{

}

template<typename... T>
static void addFile(std::vector<WavFile> &vec, std::string fileName, T... files)
{
    vec.emplace_back(WavFile{fileName});
    ASSERT_TRUE(vec.back());
    addFile(vec, files...);
}

template<typename... T>
static void BuildPackageAllPCM(const char* outName, T... toRead)
{
    //--------------------------------------------------
    // ENCODING
    //--------------------------------------------------
    std::vector<WavFile> files;

    addFile(files, toRead...);

    PackageEncoder encoder;

    for (int i = 0; i < files.size(); ++i)
    {
        encoder.AddFile(files[i], i, Encoding::PCM);
    }

    ASSERT_TRUE(encoder.WritePackage(outName) == ErrorNum::NoErrors);
}

static void playSound(char* soundName)
{
    BuildPackageAllPCM("TestFiles/TESTRealTimeAudioBank.pak", soundName);

    IO::MemoryMappedFile package("TestFiles/TESTRealTimeAudioBank.pak");
    std::unordered_map<uint64_t, SoundData> data;
    PackageDecoder::DecodePackage(data, package);

    EventManager eventManager;

    unsigned largestSize = 0;

    Filter<float> *filter = nullptr;

    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
        largestSize = std::max(iter->second.sampleCount, largestSize);
        if (iter->second.audioType == Encoding::PCM)
        {
            filter = new WavContainer<float>(iter->second);
            eventManager.AddEvent(filter);
        } else
        {
            filter = new OpusContainer<float>(iter->second);
            eventManager.AddEvent(filter);
        }
    }

    RealTimeAudio realTime(eventManager);
    realTime.Start();

    int sleepTime = data[0].sampleCount / (44100 * 2);
    std::this_thread::sleep_for(std::chrono::seconds(sleepTime));

}

TEST(RealTimePlayback, level_wav)
{
    playSound("TestFiles/level.wav");
}

#endif //I_SOUND_ENGINE_REALTIMEAUDIOMODULE_H
