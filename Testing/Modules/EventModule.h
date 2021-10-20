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
void BuildPackageAllPCM(char* outName, T... toRead)
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
void BuildPackageAllOpus(char* outName, T... toRead)
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
void BuildPackageAlternating(char * outName, T... toRead)
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

void SumAllInPackage(char* packageName, char* outFileName)
{
    IO::MemoryMappedFile package(packageName);
    std::unordered_map<uint64_t, SoundData> data;
    PackageDecoder::DecodePackage(data, package);

    EventManager eventManager;

    unsigned largestSize = 0;

    for(auto iter = data.begin(); iter != data.end(); ++iter)
    {
        largestSize = std::max(iter->second.sampleCount, largestSize);
        if(iter->second.audioType == Encoding::PCM)
        {
            WavContainer<float>* wavContainer = new WavContainer<float>(iter->second);
            eventManager.AddEvent(wavContainer);
        }
        else
        {
            OpusContainer<float>* opusContainer = new OpusContainer<float>(iter->second);
            eventManager.AddEvent(opusContainer);
        }
    }

    WavFile wav("TestFiles/level.wav");
    std::fstream tesConvert(outFileName, std::ios_base::binary | std::ios_base::out);
    RiffHeader riffHeader{{'R','I','F','F'},
                          0,
                          {'W','A','V','E'}};
    tesConvert.write(reinterpret_cast<char*>(&riffHeader), sizeof(riffHeader));
    tesConvert.write(reinterpret_cast<const char*>(&wav.GetFormat()), sizeof(FormatHeader));
    GenericHeaderChunk dataChunk{{'d', 'a','t','a'}, wav.GetDataSize()};
    dataChunk.chunkSize = wav.GetDataSize();
    tesConvert.write(reinterpret_cast<char*>(&dataChunk), sizeof(dataChunk));

    //tesConvert.write(data[0].data, wav.GetDataSize());

//    char* correctData = new char[wav.GetDataSize()];
//    wav.GetDataInNativeType(correctData);
//    tesConvert.write(correctData, wav.GetDataSize());

    for(int i = 0; i < data[0].sampleCount; ++i)
    {
        short right = static_cast<short>(data[0].data[i] * (1<<15));

        tesConvert.write(reinterpret_cast<char*>(&right), sizeof(short));
    }


    int samples = 0;
    do
    {
        Frame<float> frame = {0,0};
        samples = eventManager.GetSamplesFromAllEvents(1, &frame);
        short right = static_cast<short>(frame.rightChannel * (1<<15));
        short left = static_cast<short>(frame.leftChannel * (1<<15));

        //tesConvert.write(reinterpret_cast<char*>(&left), sizeof(short));
        //tesConvert.write(reinterpret_cast<char*>(&right), sizeof(short));
    }while(samples > 0);
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

TEST(Events, SumTwoOpusPlayback)
{
    BuildPackageAllOpus("TestFiles/TESTEventPack.pak", "TestFiles/level.wav", "TestFiles/credits.wav");
    SumAllInPackage("TestFiles/TESTEventPack.pak", "TestFiles/TESTEventOpusSum2.wav");
}


#endif //I_SOUND_ENGINE_EVENTMODULE_H
