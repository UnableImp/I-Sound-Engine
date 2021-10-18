//
// Created by zack on 9/28/21.
//

#ifndef I_SOUND_ENGINE_PACKAGEMODULE_H
#define I_SOUND_ENGINE_PACKAGEMODULE_H

#include "gtest/gtest.h"

#include "AudioPackage/PackageEncoder.h"
#include "AudioPackage/PackageDecoder.h"
#include "AudioFormats/WavFile.h"
#include "benchmark/benchmark.h"

#include "IO/IOUtility.h"

void addFile(std::vector<WavFile>&)
{

}

template<typename... T>
void addFile(std::vector<WavFile> &vec, std::string fileName, T... files)
{
    vec.emplace_back(WavFile{fileName});
    ASSERT_TRUE(vec.back());
    addFile(vec, files...);
}

void Compare( std::unordered_map<uint64_t, SoundData>&, int)
{

}

template<typename... T>
void Compare( std::unordered_map<uint64_t, SoundData>& ParsedData, int index, std::string fileName, T... rest)
{
    WavFile wavFile(fileName);
    ASSERT_TRUE(wavFile);

    char* data = new char[wavFile.GetDataSize() * 4];

    wavFile.GetDataAsFloat(reinterpret_cast<float *>(data));

    for(int i = 0; i < ParsedData[index].sampleCount; ++i)
    {
        float left = *reinterpret_cast<float*>(ParsedData[index].data + (i * sizeof(float)));
        float right = *reinterpret_cast<float*>(data + (i * sizeof(float)));
        ASSERT_FLOAT_EQ(left, right);
    }

    delete [] data;

    Compare(ParsedData, ++index, rest...);
}

template<typename... T>
void TestPackages(std::string outName, T... toRead)
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

    //--------------------------------------------------
    // DECODING
    //--------------------------------------------------

    std::unordered_map<uint64_t, SoundData> ParsedData;
    char* dataPointer;
    PackageDecoder::DecodePackage(ParsedData, &dataPointer, outName);

    //delete [] dataPointer;

    Compare(ParsedData, 0, toRead...);

}

template<typename... T>
void TestOpusPackages(std::string outName, T... toRead)
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

    //--------------------------------------------------
    // DECODING
    //--------------------------------------------------

    std::unordered_map<uint64_t, SoundData> ParsedData;
    char* dataPointer;
    PackageDecoder::DecodePackage(ParsedData, &dataPointer, outName);

    for(int i = 0;  i < files.size(); ++i)
    {
        ASSERT_TRUE(files[i].GetFormat().channel_count == (int)ParsedData[i].channels) << "Wav " << files[i].GetFormat().channel_count << "  opus " <<  (int)ParsedData[i].channels;
        ASSERT_TRUE(files[i].GetFormat().sampling_rate == ParsedData[i].sampleRate);
    }


}



TEST(PackageWav, EncodeSimpleWav)
{
    TestPackages("TestFiles/TESTBank1Wav.pak", "TestFiles/16_bit_simple.wav");
}

TEST(PackageWav, EncodeComplexWav)
{
    TestPackages("TestFiles/TESTBank1Wav.pak", "TestFiles/16_bit_reaper.wav");
}

TEST(PackageWav, Encode2SimpleWav)
{
    TestPackages("TestFiles/TESTBank1Wav.pak", "TestFiles/16_bit_simple.wav", "TestFiles/16_bit_simple.wav");
}

TEST(PackageWav, Encode2ComplexWav)
{
    TestPackages("TestFiles/TESTBank1Wav.pak", "TestFiles/16_bit_reaper.wav", "TestFiles/16_bit_reaper.wav");
}

TEST(PackageWav, Encode2DifferntWav)
{
    TestPackages("TestFiles/TESTBank1Wav.pak", "TestFiles/16_bit_simple.wav", "TestFiles/16_bit_reaper.wav");
}

TEST(PackageWav, Encode2DiffertBitRate)
{
    TestPackages("TestFiles/TESTBank1Wav.pak", "TestFiles/8_bit_simple.wav", "TestFiles/16_bit_simple.wav");
}

TEST(PackageWav, Encode10)
{
    TestPackages("TestFiles/TESTBank1Wav.pak", "TestFiles/16_bit_reaper.wav",
                 "TestFiles/16_bit_reaper.wav", "TestFiles/16_bit_reaper.wav",
                 "TestFiles/16_bit_reaper.wav", "TestFiles/16_bit_reaper.wav",
                 "TestFiles/16_bit_reaper.wav", "TestFiles/16_bit_reaper.wav",
                 "TestFiles/16_bit_reaper.wav", "TestFiles/16_bit_reaper.wav",
                 "TestFiles/16_bit_reaper.wav");
}
#endif //I_SOUND_ENGINE_PACKAGEMODULE_H

TEST(PackageOpus, Encode144100)
{
    TestOpusPackages("TestFiles/TESTBankOpus1.pck", "TestFiles/level.wav");
}
TEST(PackageOpus, Encode148000)
{
    TestOpusPackages("TestFiles/TESTBankOpus1.pck", "TestFiles/credits.wav");
}

TEST(PackageOpus, EncodeBoth)
{
    TestOpusPackages("TestFiles/TESTBankOpus2.pck", "TestFiles/credits.wav", "TestFiles/level.wav");
}

static void Encode100WavExpectedFilePack(benchmark::State& state)
{
    for(auto _ : state)
    {
        PackageEncoder encoder;
        WavFile wav("TestFiles/Slash2.wav");
        for(int i = 0; i < 100; ++i)
        {
            encoder.AddFile(wav, i, PCM);
        }
        encoder.WritePackage("TestFiles/TEST100WavFilesExpected.pak");
    }
}
BENCHMARK(Encode100WavExpectedFilePack);

static void Encode100WavBrutalFilePack(benchmark::State& state)
{
    for(auto _ : state)
    {
        PackageEncoder encoder;
        WavFile wav("TestFiles/level.wav");
        for(int i = 0; i < 100; ++i)
        {
            encoder.AddFile(wav, i, PCM);
        }
        encoder.WritePackage("TestFiles/TEST100WavFilesBrutal.pak");
    }
}
BENCHMARK(Encode100WavBrutalFilePack);

static void Read1_100FilePackExpected(benchmark::State& state)
{
    std::string testPakFilepath = "TestFiles/TEST100WavFilesExpected.pak";
    if (!IO::FileExists(testPakFilepath))
    {
      PackageEncoder encoder;
      WavFile wav("TestFiles/Slash2.wav");
      for (int i = 0; i < 100; ++i)
      {
        encoder.AddFile(wav, i, PCM);
      }
      encoder.WritePackage(testPakFilepath);
    }

    for (auto _ : state)
    {
        std::unordered_map<uint64_t, SoundData> ParsedData;
        char* dataPointer;
        PackageDecoder::DecodePackage(ParsedData, &dataPointer, testPakFilepath);
        benchmark::DoNotOptimize(ParsedData);
        delete [] dataPointer;
    }
}
BENCHMARK(Read1_100FilePackExpected);

static void Read1_100FilePackBrutal(benchmark::State& state)
{
    std::string testPakFilepath = "TestFiles/TEST100WavFilesBrutal.pak";
    if (!IO::FileExists(testPakFilepath))
    {
        PackageEncoder encoder;
        WavFile wav("TestFiles/level.wav");
        for (int i = 0; i < 100; ++i)
        {
            encoder.AddFile(wav, i, PCM);
        }
        encoder.WritePackage(testPakFilepath);
    }

    for (auto _ : state)
    {
        std::unordered_map<uint64_t, SoundData> ParsedData;
        char* dataPointer;
        PackageDecoder::DecodePackage(ParsedData, &dataPointer, testPakFilepath);
        benchmark::DoNotOptimize(ParsedData);
        delete [] dataPointer;
    }
}
BENCHMARK(Read1_100FilePackBrutal);

static void EncodeLevel44100ToOpus(benchmark::State& state)
{
    WavFile file("TestFiles/level.wav");
    char *buffer = new char[file.GetDataSize()];
    for (auto _: state)
    {
        file.GetDataAsOpus(buffer);
    }
}
BENCHMARK(EncodeLevel44100ToOpus);

static void EncodeCredits48000ToOpus(benchmark::State& state)
{
    WavFile file("TestFiles/credits.wav");
    char *buffer = new char[file.GetDataSize()];
    for (auto _: state)
    {
        file.GetDataAsOpus(buffer);
    }
}
BENCHMARK(EncodeCredits48000ToOpus);

static void Encode100Opus(benchmark::State& state)
{
    for(auto _ : state)
    {
        PackageEncoder encoder;
        WavFile wav("TestFiles/level.wav");
        for(int i = 0; i < 100; ++i)
        {
            encoder.AddFile(wav, i, Opus);
        }
        encoder.WritePackage("TestFiles/TEST100WavFilesOpusBrutal.pak");
    }
}
BENCHMARK(Encode100Opus);

static void Read1_100OpusPack(benchmark::State& state)
{
    std::string testPakFilepath = "TestFiles/TEST100WavFilesOpusBrutal.pak";
    if (!IO::FileExists(testPakFilepath))
    {
      PackageEncoder encoder;
      WavFile wav("TestFiles/level.wav");
      for (int i = 0; i < 100; ++i)
      {
        encoder.AddFile(wav, i, Opus);
      }
      encoder.WritePackage(testPakFilepath);
    }
    for (auto _ : state)
    {
        std::unordered_map<uint64_t, SoundData> ParsedData;
        char* dataPointer;
        PackageDecoder::DecodePackage(ParsedData, &dataPointer, testPakFilepath);
        benchmark::DoNotOptimize(ParsedData);
        delete [] dataPointer;
    }
}
BENCHMARK(Read1_100OpusPack);