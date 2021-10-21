//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_EVENTMODULE_H
#define I_SOUND_ENGINE_EVENTMODULE_H

#include "gtest/gtest.h"
#include "benchmark/benchmark.h"
#include "Events/Event.h"
#include "Events/EventManager.h"

#include "AudioFormats/WavFile.h"
#include "AudioPackage/PackageEncoder.h"
#include "AudioPackage/PackageDecoder.h"
#include "Filters/WavContainer.h"
#include "Filters/OpusContainer.h"


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

template<typename... T>
static void BuildPackageAllOpus(const char* outName, T... toRead)
{
    //--------------------------------------------------
    // ENCODING
    //--------------------------------------------------
    std::vector<WavFile> files;

    addFile(files, toRead...);

    PackageEncoder encoder;

    for (int i = 0; i < files.size(); ++i)
    {
        encoder.AddFile(files[i], i, Encoding::Opus);
    }

    ASSERT_TRUE(encoder.WritePackage(outName) == ErrorNum::NoErrors);
}

template<typename... T>
static  void BuildPackageAlternating(const char * outName, T... toRead)
{
    //--------------------------------------------------
    // ENCODING
    //--------------------------------------------------
    std::vector<WavFile> files;

    addFile(files, toRead...);

    PackageEncoder encoder;

    for (int i = 0; i < files.size(); ++i)
    {
        encoder.AddFile(files[i], i, i % 2 ? Encoding::PCM : Encoding::Opus);
    }

    ASSERT_TRUE(encoder.WritePackage(outName) == ErrorNum::NoErrors);
}

static void SumAllInPackage(const char* packageName, const char* outFileName)
{
    IO::MemoryMappedFile package(packageName);
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

    WavFile wav("TestFiles/level.wav");
    std::fstream tesConvert(outFileName, std::ios_base::binary | std::ios_base::out);
    RiffHeader riffHeader{{'R', 'I', 'F', 'F'},
                          0,
                          {'W', 'A', 'V', 'E'}};
    tesConvert.write(reinterpret_cast<char *>(&riffHeader), sizeof(riffHeader));
    tesConvert.write(reinterpret_cast<const char *>(&wav.GetFormat()), sizeof(FormatHeader));
    GenericHeaderChunk dataChunk{{'d', 'a', 't', 'a'}, wav.GetDataSize()};
    dataChunk.chunkSize = wav.GetDataSize();
    tesConvert.write(reinterpret_cast<char *>(&dataChunk), sizeof(dataChunk));

    //tesConvert.write(data[0].data, wav.GetDataSize());

//    char* correctData = new char[wav.GetDataSize()];
//    wav.GetDataInNativeType(correctData);
//    tesConvert.write(correctData, wav.GetDataSize());

    int samples = 0;
    do
    {
        Frame<float> frame = {0, 0};
        samples = eventManager.GetSamplesFromAllEvents(1, &frame);
        short right = static_cast<short>(frame.rightChannel * (1 << 15));
        short left = static_cast<short>(frame.leftChannel * (1 << 15));

        tesConvert.write(reinterpret_cast<char *>(&left), sizeof(short));
        tesConvert.write(reinterpret_cast<char *>(&right), sizeof(short));
    } while (samples > 0);

    delete filter;
}

void SumAllInPackageNoFileIo(std::unordered_map<uint64_t, SoundData>& data, Frame<float>* buf, int bufSize)
{
    EventManager eventManager;

    unsigned largestSize = 0;

    Filter<float> *filter = nullptr;

    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
        for(int i = 0; i < 100; ++i)
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
    }

    for(int i = 0; i < largestSize; i += bufSize)
    {
        eventManager.GetSamplesFromAllEvents(bufSize, buf);
    }

}

TEST(Events, WavPlayback)
{
    BuildPackageAllPCM("TestFiles/TESTEventPack.pak", "TestFiles/level.wav");
    SumAllInPackage("TestFiles/TESTEventPack.pak", "TestFiles/TESTEventPCMSum.wav");
}

TEST(Events, OpusPlayback)
{
    BuildPackageAllOpus("TestFiles/TESTEventPack.pak", "TestFiles/level.wav");
    SumAllInPackage("TestFiles/TESTEventPack.pak", "TestFiles/TESTEventOpusSum.wav");
}

TEST(Events, SumTwoWavPlayback)
{
    BuildPackageAllPCM("TestFiles/TESTEventPack.pak", "TestFiles/level.wav", "TestFiles/credits.wav");
    SumAllInPackage("TestFiles/TESTEventPack.pak", "TestFiles/TESTEventWavSum2.wav");
}

TEST(Events, SumTwoOpusPlayback)
{
    BuildPackageAllOpus("TestFiles/TESTEventPack.pak", "TestFiles/level.wav", "TestFiles/credits.wav");
    SumAllInPackage("TestFiles/TESTEventPack.pak", "TestFiles/TESTEventOpusSum2.wav");
}

TEST(Events, SumTwoBothPlayback)
{
    BuildPackageAlternating("TestFiles/TESTEventPack.pak", "TestFiles/level.wav", "TestFiles/credits.wav");
    SumAllInPackage("TestFiles/TESTEventPack.pak", "TestFiles/TESTEventBothSum2.wav");
}

static void readEventFromBuffer(benchmark::State& state, int bufSize)
{
    BuildPackageAllPCM("TestFiles/TESTEventPack.pak", "TestFiles/Slash2.wav");
    IO::MemoryMappedFile package("TestFiles/TESTEventPack.pak");
    std::unordered_map<uint64_t, SoundData> data;
    PackageDecoder::DecodePackage(data, package);

    Frame<float>* buf = new Frame<float>[bufSize];

    //SumAllInPackageNoFileIo(data, buf, bufSize);

    for (auto _ : state)
    {
        SumAllInPackageNoFileIo(data, buf, bufSize);
    }
    delete [] buf;
}

static void Read100WavEventsSong1Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 1);
}
BENCHMARK(Read100WavEventsSong1Buf);

static void Read100WavEventsSong4Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 4);
}
BENCHMARK(Read100WavEventsSong4Buf);

static void Read100WavEventsSong8Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 8);
}
BENCHMARK(Read100WavEventsSong8Buf);

static void Read100WavEventsSong16Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 16);
}
BENCHMARK(Read100WavEventsSong16Buf);

static void Read100WavEventsSong32Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 32);
}
BENCHMARK(Read100WavEventsSong32Buf);

static void Read100WavEventsSong64Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 64);
}
BENCHMARK(Read100WavEventsSong64Buf);

static void Read100WavEventsSong128Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 128);
}
BENCHMARK(Read100WavEventsSong128Buf);

static void Read100WavEventsSong256Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 256);
}
BENCHMARK(Read100WavEventsSong256Buf);

static void Read100WavEventsSong512Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 512);
}
BENCHMARK(Read100WavEventsSong512Buf);

static void Read100WavEventsSong882Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 882);
}
BENCHMARK(Read100WavEventsSong882Buf);

static void Read100WavEventsSong1024Buf(benchmark::State& state)
{
    readEventFromBuffer(state, 1024);
}
BENCHMARK(Read100WavEventsSong1024Buf);


#endif //I_SOUND_ENGINE_EVENTMODULE_H
