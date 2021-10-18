//
// Created by zack on 9/26/21.
//

#ifndef I_SOUND_ENGINE_CONTAINERMODULE_H
#define I_SOUND_ENGINE_CONTAINERMODULE_H

#include "AudioFormats/OpusEncoderWrapper.h"
#include "gtest/gtest.h"
#include "AudioFormats/WavFile.h"
#include "benchmark/benchmark.h"
#include "Filters/OpusContainer.h"

void readWaveFile(std::string name);

#endif //I_SOUND_ENGINE_CONTAINERMODULE_H


TEST(WavFiles, Simple_8_bit_read)
{
    readWaveFile("TestFiles/8_bit_simple.wav");

}
TEST(WavFiles, Complex_8_bit_read)
{
    readWaveFile("TestFiles/8_bit_reaper.wav");
}

TEST(WavFiles, Simple_16_bit_read)
{
    readWaveFile("TestFiles/16_bit_simple.wav");

}
TEST(WavFiles, Complex_16_bit_read)
{
    readWaveFile("TestFiles/16_bit_reaper.wav");
}

TEST(WavFiles, Complex_24_bit_read)
{
    //readWaveFile("TestFiles/24_bit_reaper.wav");
}

TEST(WavFiles, Complex_32_bit_read)
{
    //readWaveFile("TestFiles/32_bit_reaper.wav");
}

void encodeWaveToOggThenWav(const char* inFile, const char* outFileOgg, const char*  outFileWav);

TEST(OpusFiles, EncodeCredits48000)
{
    encodeWaveToOggThenWav("TestFiles/credits.wav", "TestFiles/TESTcredits.ogg", "TestFiles/TESTcredits.wav");
}

TEST(OpusFiles, EncodeLevel44100)
{
    encodeWaveToOggThenWav("TestFiles/level.wav", "TestFiles/TESTlevel.ogg", "TestFiles/TESTlevel.wav");
}

static void Read100WavFilesExpected(benchmark::State& state)
{
    // Not timed
    for (auto _ : state)
    {
        char* bufferOfBuffers[100];
        for(int i = 0; i < 100; ++i)
        {
            WavFile wavFile("TestFiles/Slash2.wav");
            bufferOfBuffers[i] = new char[(wavFile.GetDataSize() / 2) * sizeof(float)];
            wavFile.GetDataAsFloat(reinterpret_cast<float*>(bufferOfBuffers[i]));
        }
        for(int i = 0; i < 100; ++i)
        {
            delete [] bufferOfBuffers[i];
        }
    }

}
BENCHMARK(Read100WavFilesExpected);

static void Read100WavFilesBrutal(benchmark::State& state)
{
    // Not timed
    for (auto _ : state)
    {
        char* bufferOfBuffers[100];
        for(int i = 0; i < 100; ++i)
        {
            WavFile wavFile("TestFiles/level.wav");
            bufferOfBuffers[i] = new char[(wavFile.GetDataSize() / 2) * sizeof(float)];
            wavFile.GetDataAsFloat(reinterpret_cast<float*>(bufferOfBuffers[i]));
        }
        for(int i = 0; i < 100; ++i)
        {
            delete [] bufferOfBuffers[i];
        }
    }

}
BENCHMARK(Read100WavFilesBrutal);
