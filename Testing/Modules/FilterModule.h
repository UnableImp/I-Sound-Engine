//
// Created by zack on 10/27/21.
//

#ifndef I_SOUND_ENGINE_FILTERMODULE_H
#define I_SOUND_ENGINE_FILTERMODULE_H

#include "gtest/gtest.h"
#include "benchmark/benchmark.h"
#include "Events/Event.h"
#include "Events/EventManager.h"
#include <array>

#include "AudioFormats/WavFile.h"
#include "AudioPackage/PackageEncoder.h"
#include "AudioPackage/PackageDecoder.h"
#include "Filters/WavContainer.h"
#include "Filters/OpusContainer.h"
#include "Filters/ConvolutionFreq.h"
#include "RealTimeParameters/GameObjectManager.h"

#include "Filters/Biqaud/DualBiquad.h"
#include "Filters/Biqaud/FirstOrderLowpass.h"
#include "Filters/Biqaud/SecondOrderLowpass.h"
#include "Filters/Biqaud/LinkwitzRileyLowpass.h"

#include "Filters/DistanceAttenuation.h"

#include "Filters/DualFilter.h"

#include <filesystem>

#include <immintrin.h>

#include <bitset>

#include <pffft.hpp>

#include "Filters/ITD.h"

template<typename sampleType>
inline static sampleType lerp(sampleType a, sampleType b, float t)
{
    return a+(t*(b-a));
}

inline static std::complex<float> lerp(std::complex<float>& a, std::complex<float>& b, float t)
{
    return std::polar(lerp(std::abs(a), std::abs(b), t), lerp(std::arg(a), std::arg(b), t));
}

static void lerp(std::complex<float>* a, std::complex<float>* b, float t, std::complex<float>* out, int cnt)
{
    for(int i = 0; i < cnt; ++i)
    {
        out[i] = lerp(a[i], b[i], t);
    }
}

static void addFile(std::vector<WavFile>&)
{

}

template<typename... T>
static void addFile(std::vector<WavFile> &vec, std::string fileName, T... files)
{
    vec.emplace_back(WavFile{fileName});
    ASSERT_TRUE(vec.back()) << " " << fileName << " " <<  vec.back().GetError();
    addFile(vec, files...);
}

template<typename... T>
static void BuildPackageAllPCM(int offset, const char* outName, T... toRead)
{
    //--------------------------------------------------
    // ENCODING
    //--------------------------------------------------
    std::vector<WavFile> files;

    addFile(files, toRead...);

    PackageEncoder encoder;

    for (int i = 0; i < files.size(); ++i)
    {
        encoder.AddFile(files[i], i+offset, Encoding::PCM);
    }

    ASSERT_TRUE(encoder.WritePackage(outName) == ErrorNum::NoErrors);
}

template<typename... T>
static void BuildPackageAllOpus(int offset, const char* outName, T... toRead)
{
    //--------------------------------------------------
    // ENCODING
    //--------------------------------------------------
    std::vector<WavFile> files;

    addFile(files, toRead...);

    PackageEncoder encoder;

    for (int i = 0; i < files.size(); ++i)
    {
        encoder.AddFile(files[i], i + offset, Encoding::Opus);
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


static void PhaseAlignSignal(float* signal)
{
    int offset = 0;

    //-----------------------------------
    // Left ear
    //-----------------------------------
    for(int i = 0; i < blockSize; ++i)
    {
        if(std::abs(signal[i]) > delta)
        {
            offset = i;
            break;
        }
    }
    for(int i = offset, j = 0; i < blockSize; ++i, ++j)
    {
        std::swap(signal[i], signal[j]);
    }


}

static void simulateEventManager(EventManager& eventManager, const char* outFileName, int frameSize)
{
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

    Frame<float>* frame = new Frame<float>[frameSize]();
    int samples = 0;
    do
    {
        samples = eventManager.GetSamplesFromAllEvents(frameSize, frame);
        for(int i = 0; i < frameSize; ++i)
        {
            short right = static_cast<short>(frame[i].rightChannel * (1 << 15));
            short left = static_cast<short>(frame[i].leftChannel * (1 << 15));

            tesConvert.write(reinterpret_cast<char *>(&left), sizeof(short));
            tesConvert.write(reinterpret_cast<char *>(&right), sizeof(short));
        }
    } while (samples > 0);
    delete [] frame;
}

static void simulateEventManagerDecreasingCutoff(EventManager& eventManager, const char* outFileName, int frameSize, DualBiquad<float>* qaud)
{
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

    Frame<float>* frame = new Frame<float>[frameSize]();
    int samples = 0;
    int cutoff = 2000;
    do
    {
        qaud->SetCutoff(cutoff);
        cutoff -= 7;
        samples = eventManager.GetSamplesFromAllEvents(frameSize, frame);
        for(int i = 0; i < frameSize; ++i)
        {
            short right = static_cast<short>(frame[i].rightChannel * (1 << 15));
            short left = static_cast<short>(frame[i].leftChannel * (1 << 15));

            tesConvert.write(reinterpret_cast<char *>(&left), sizeof(short));
            tesConvert.write(reinterpret_cast<char *>(&right), sizeof(short));
        }
    } while (samples > 0);
    delete [] frame;
}

static void simulateEventManagerWithCalulator(EventManager& eventManager, const char* outFileName, int frameSize, int id, int jump, GameObjectManager& objManager, float speed)
{
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

    Frame<float>* frame = new Frame<float>[frameSize]();
    float angle = 0;
    int samples = 0;
    int totalSamples = 0;
    do
    {
        samples = eventManager.GetSamplesFromAllEvents(frameSize, frame);
        for(int i = 0; i < frameSize; ++i)
        {
            short right = static_cast<short>(frame[i].rightChannel * (1 << 15));
            short left = static_cast<short>(frame[i].leftChannel * (1 << 15));

            tesConvert.write(reinterpret_cast<char *>(&left), sizeof(short));
            tesConvert.write(reinterpret_cast<char *>(&right), sizeof(short));
        }

        totalSamples += samples;
        {
            angle += speed;

            IVector3 newPos{std::cos(angle * 3.145f / 180.0f) * 3, 0,std::sin(angle * 3.145f / 180.0f) * 3};
            objManager.SetGameObjectPosition(id, newPos);
        }

    } while (samples > 0);
    delete [] frame;
}

static void simulateEventManagerWithElevation(EventManager& eventManager, const char* outFileName, int frameSize, int id, int start, GameObjectManager& objManager, float speed)
{
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

    Frame<float>* frame = new Frame<float>[frameSize]();
    float angle = 0;
    int samples = 0;
    int totalSamples = 0;
    do
    {
        samples = eventManager.GetSamplesFromAllEvents(frameSize, frame);
        for(int i = 0; i < frameSize; ++i)
        {
            short right = static_cast<short>(frame[i].rightChannel * (1 << 15));
            short left = static_cast<short>(frame[i].leftChannel * (1 << 15));

            tesConvert.write(reinterpret_cast<char *>(&left), sizeof(short));
            tesConvert.write(reinterpret_cast<char *>(&right), sizeof(short));
        }

        totalSamples += samples;
        {
            angle += speed;

            GameObject obj;
            objManager.GetGameObject(10, obj);
            IVector3 curPos = obj.GetPosition();
            IVector3 newPos{curPos.x, curPos.y + speed, curPos.z};
            objManager.SetGameObjectPosition(id, newPos);
        }

    } while (samples > 0);
    delete [] frame;
}

static void simulateEventManagerWithDirection(EventManager& eventManager, const char* outFileName, int frameSize, int id, GameObjectManager& objManager, IVector3 dir)
{
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

    Frame<float>* frame = new Frame<float>[frameSize]();
    float angle = 0;
    int samples = 0;
    int totalSamples = 0;
    do
    {
        samples = eventManager.GetSamplesFromAllEvents(frameSize, frame);
        for(int i = 0; i < frameSize; ++i)
        {
            short right = static_cast<short>(frame[i].rightChannel * (1 << 15));
            short left = static_cast<short>(frame[i].leftChannel * (1 << 15));

            tesConvert.write(reinterpret_cast<char *>(&left), sizeof(short));
            tesConvert.write(reinterpret_cast<char *>(&right), sizeof(short));
        }

        totalSamples += samples;
        GameObject obj;
        objManager.GetGameObject(id, obj);
        IVector3 newPos = obj.GetPosition();
        objManager.SetGameObjectPosition(id, newPos + dir);


    } while (samples > 0);
    delete [] frame;
}

static void SumAllInPackageWithFFT(const char* packageName, const char* outFileName, int frameSize)
{
    IO::MemoryMappedFile package(packageName);
//    std::unordered_map<uint64_t, SoundData> data;
//    PackageDecoder::DecodePackage(data, package);
    PackageManager data;
    data.LoadPack(packageName);

    GameObjectManager objectManager;
    EventManager eventManager(data,objectManager);

    unsigned largestSize = 0;

    Filter<float> *filter = nullptr;

    for (auto iter = data.GetSounds().begin(); iter != data.GetSounds().end(); ++iter)
    {
        largestSize = std::max(iter->second.sampleCount, largestSize);
        if (iter->second.audioType == Encoding::PCM)
        {
            filter = new WavContainer<float>(iter->second);

        } else
        {
            filter = new OpusContainer<float>(iter->second);

        }
        //eventManager.AddEvent(filter, new ConvolutionFreq(frameSize));
    }

    simulateEventManager(eventManager, outFileName, frameSize);
}


static std::string WriteToWav(const std::filesystem::path& path)
{
    WavFile wav("TestFiles/level.wav");

    std::string outFile = path.string();
    outFile = outFile.substr(0, outFile.size() - 3);
    outFile += "wav";

    //std::cout << outFile << " " << path << std::endl;

    std::fstream tesConvert(outFile.c_str(), std::ios_base::binary | std::ios_base::out);
    RiffHeader riffHeader{{'R', 'I', 'F', 'F'},
                          0,
                          {'W', 'A', 'V', 'E'}};
    tesConvert.write(reinterpret_cast<char *>(&riffHeader), sizeof(riffHeader));
    FormatHeader fmt = wav.GetFormat();
    fmt.channel_count = 1;
    tesConvert.write(reinterpret_cast<const char *>(&fmt), sizeof(FormatHeader));
    GenericHeaderChunk dataChunk{{'d', 'a', 't', 'a'}, wav.GetDataSize()};
    dataChunk.chunkSize = 1024;
    tesConvert.write(reinterpret_cast<char *>(&dataChunk), sizeof(dataChunk));

    std::fstream tesConver2(outFile.c_str(), std::ios_base::binary | std::ios_base::out);

    tesConver2.write(reinterpret_cast<char *>(&riffHeader), sizeof(riffHeader));
    tesConver2.write(reinterpret_cast<const char *>(&fmt), sizeof(FormatHeader));
    tesConver2.write(reinterpret_cast<char *>(&dataChunk), sizeof(dataChunk));

    std::fstream readFrom(path.c_str(), std::ios_base::binary | std::ios_base::in);

    short buffer[512];
    readFrom.read(reinterpret_cast<char*>(buffer), 1024);

    short outBuf[512];

    for(int i = 0; i < 512; i++)
    {
        short v = (buffer[i] >> 8) | (buffer[i] << 8);
        outBuf[i] = v;

        tesConvert.write(reinterpret_cast<char*>(&v), sizeof(short));
    }

    return outFile;
}

static void WriteFileFromBuffer(const float* outData, std::string& path )
{
    WavFile wav("TestFiles/level.wav");
    const std::string& outFile = path;

    //std::cout << outFile << " " << path << std::endl;

    std::fstream tesConvert(outFile.c_str(), std::ios_base::binary | std::ios_base::out);
    RiffHeader riffHeader{{'R', 'I', 'F', 'F'},
                          0,
                          {'W', 'A', 'V', 'E'}};
    tesConvert.write(reinterpret_cast<char *>(&riffHeader), sizeof(riffHeader));
    FormatHeader fmt = wav.GetFormat();
    fmt.channel_count = 1;
    tesConvert.write(reinterpret_cast<const char *>(&fmt), sizeof(FormatHeader));
    GenericHeaderChunk dataChunk{{'d', 'a', 't', 'a'}, wav.GetDataSize()};
    dataChunk.chunkSize = 1024;
    tesConvert.write(reinterpret_cast<char *>(&dataChunk), sizeof(dataChunk));

    std::fstream tesConver2(outFile.c_str(), std::ios_base::binary | std::ios_base::out);

    tesConver2.write(reinterpret_cast<char *>(&riffHeader), sizeof(riffHeader));
    tesConver2.write(reinterpret_cast<const char *>(&fmt), sizeof(FormatHeader));
    tesConver2.write(reinterpret_cast<char *>(&dataChunk), sizeof(dataChunk));

    for(int i = 0; i < 512; i++)
    {
        short v = outData[i] * (1<<15);

        tesConvert.write(reinterpret_cast<char*>(&v), sizeof(short));
    }
}

static void Normalize(float* buffer, int length)
{
    for(int i = 0; i < length; ++i)
        buffer[i] /= length;
}

static void LerpFromBuffer(uint64_t isRight, float* buf1Real, float* buf2Real, std::filesystem::path& path1, PackageEncoder& encoder)
{
    int size = 1024;
    int lerpcnt = 1024;

    std::complex<float>* complexOf1 = new std::complex<float>[size]();
    std::complex<float>* complexOf2 = new std::complex<float>[size]();
    std::complex<float>* complexAof1 = new std::complex<float>[size]();
    std::complex<float>* complexAof2 = new std::complex<float>[size]();
    std::complex<float>* complexOfOut = new std::complex<float>[size]();

    float* buf1 = new float[size]();
    float* buf2 = new float[size]();
    float* aligned1 = new float[size]();
    float* aligned2 = new float[size]();

    memcpy(buf1, buf1Real, sizeof(float) * 512);
    memcpy(buf2, buf2Real, sizeof(float) * 512);
    memcpy(aligned1, buf1Real, sizeof(float) * 512);
    memcpy(aligned2, buf2Real, sizeof(float) * 512);

    PhaseAlignSignal(aligned1);
    PhaseAlignSignal(aligned2);

    float*  out = new float[size]();

    pffft::Fft<float> fft(size);

    fft.forward(buf1, complexOf1);
    fft.forward(buf2, complexOf2);
    fft.forward(aligned1, complexAof1);
    fft.forward(aligned2, complexAof2);

    std::string startOfPath = path1.string().substr(0, path1.string().length() - 5);
    //std::cout << startOfPath << " " << startOfPath.substr(startOfPath.length() - 3, startOfPath.length()) << std::endl;
    //----------------------------------------------------------------------------------
    //TODO need to be rewriten to support any gaps not just 5
    //---------------------------------------------------------------------------------
    uint64_t number = std::stoi(startOfPath.substr(startOfPath.length() - 3, startOfPath.length()));

    startOfPath = startOfPath.substr(0, startOfPath.length() - 3);
    if(number < 100)
        startOfPath += "0";
    if(number < 10)
        startOfPath += "0";
    //--------------------------------------------------------------------------------
    std::string writeFile = std::string(startOfPath + std::to_string(number) + std::string("aAligned.wav"));
    WriteFileFromBuffer(aligned1, writeFile);
    uint64_t id = number << 32;
    id |= static_cast<uint64_t>(1) << 52;
    id |= static_cast<uint64_t>(isRight) << 51;
    id |= static_cast<uint64_t>(1) << 55;
    encoder.AddFile(writeFile, id, Encoding::PCM);

    //---------------------------------------------------------------------------------
    lerp(complexOf1, complexOf2, 1.0f / 5.0f, complexOfOut, lerpcnt);
    fft.inverse(complexOfOut, out);
    Normalize(out, size);

    ++number;
    writeFile = std::string(startOfPath + std::to_string(number) + std::string("a.wav"));
    WriteFileFromBuffer(out, writeFile);
    id = number << 32;
    id |= static_cast<uint64_t>(1) << 52;
    id |= static_cast<uint64_t>(isRight) << 51;
    encoder.AddFile(writeFile, id, Encoding::PCM);

    lerp(complexAof1, complexAof2, 1.0f / 5.0f, complexOfOut, lerpcnt);
    fft.inverse(complexOfOut, out);
    Normalize(out, size);

    writeFile = std::string(startOfPath + std::to_string(number) + std::string("aAligned.wav"));
    WriteFileFromBuffer(out, writeFile);
    id |= static_cast<uint64_t>(1) << 55;
    encoder.AddFile(writeFile, id, Encoding::PCM);

    //---------------------------------------------------------------------------------
    lerp(complexOf1, complexOf2, 2.0f/ 5.0f, complexOfOut, lerpcnt);
    fft.inverse(complexOfOut, out);
    Normalize(out, size);

    ++number;
    writeFile = std::string(startOfPath + std::to_string(number) + std::string("a.wav"));
    WriteFileFromBuffer(out, writeFile);
    id = number << 32;
    id |= static_cast<uint64_t>(1) << 52;
    id |= static_cast<uint64_t>(isRight) << 51;
    encoder.AddFile(writeFile, id, Encoding::PCM);

    lerp(complexAof1, complexAof2, 2.0f / 5.0f, complexOfOut, lerpcnt);
    fft.inverse(complexOfOut, out);
    Normalize(out, size);

    writeFile = std::string(startOfPath + std::to_string(number) + std::string("aAligned.wav"));
    WriteFileFromBuffer(out, writeFile);
    id |= static_cast<uint64_t>(1) << 55;
    encoder.AddFile(writeFile, id, Encoding::PCM);

    //---------------------------------------------------------------------------------
    lerp(complexOf1, complexOf2, 3.0f / 5.0f, complexOfOut, lerpcnt);
    fft.inverse(complexOfOut, out);
    Normalize(out, size);

    ++number;
    writeFile = std::string(startOfPath + std::to_string(number) + std::string("a.wav"));
    WriteFileFromBuffer(out, writeFile);
    id = number << 32;
    id |= static_cast<uint64_t>(1) << 52;
    id |= static_cast<uint64_t>(isRight) << 51;
    encoder.AddFile(writeFile, id, Encoding::PCM);

    lerp(complexAof1, complexAof2, 3.0f / 5.0f, complexOfOut, lerpcnt);
    fft.inverse(complexOfOut, out);
    Normalize(out, size);

    writeFile = std::string(startOfPath + std::to_string(number) + std::string("aAligned.wav"));
    WriteFileFromBuffer(out, writeFile);
    id |= static_cast<uint64_t>(1) << 55;
    encoder.AddFile(writeFile, id, Encoding::PCM);

    //---------------------------------------------------------------------------------
    lerp(complexOf1, complexOf2, 4.0f / 5.0f, complexOfOut, lerpcnt);
    fft.inverse(complexOfOut, out);
    Normalize(out, size);

    ++number;
    writeFile = std::string(startOfPath + std::to_string(number) + std::string("a.wav"));
    WriteFileFromBuffer(out, writeFile);
    id = number << 32;
    id |= static_cast<uint64_t>(1) << 52;
    id |= static_cast<uint64_t>(isRight) << 51;
    encoder.AddFile(writeFile, id, Encoding::PCM);

    lerp(complexAof1, complexAof2, 4.0f / 5.0f, complexOfOut, lerpcnt);
    fft.inverse(complexOfOut, out);
    Normalize(out, size);

    writeFile = std::string(startOfPath + std::to_string(number) + std::string("aAligned.wav"));
    WriteFileFromBuffer(out, writeFile);
    id |= static_cast<uint64_t>(1) << 55;
    encoder.AddFile(writeFile, id, Encoding::PCM);

    //--------------------------------------------------------------------------------------------------
//    ++number;
//    writeFile = std::string(startOfPath + std::to_string(number) + std::string("aAligned.wav"));
//    WriteFileFromBuffer(aligned2, writeFile);
//    id = number << 32;
//    id |= static_cast<uint64_t>(1) << 52;
//    id |= static_cast<uint64_t>(isRight) << 51;
//    id |= static_cast<uint64_t>(1) << 55;
//    encoder.AddFile(writeFile, id, Encoding::PCM);

    delete [] complexOf1;
    delete [] complexOf2;
    delete [] complexAof1;
    delete [] complexAof2;
    delete [] complexOfOut;
    delete [] buf1;
    delete [] buf2;
    delete [] aligned1;
    delete [] aligned2;
    delete [] out;
}

static void LerpMissingHRIR(PackageEncoder& encoder, std::string path)
{
    std::unordered_map<uint64_t, SoundData> sounds;
    IO::MemoryMappedFile file(path);
    PackageDecoder::DecodePackage(sounds, file);


    for(uint64_t i = 0; i < 360; i+=5)
    {
        //-------------------------------------------
        // Left Ear
        //------------------------------------------

        uint64_t curId = i << 32;
        //Evelavation is 0 for now
        curId |= static_cast<uint64_t>(1) << 52;

        uint64_t nextId = (i + 5 < 360 ? i + 5 : 0) << 32;
        nextId |= static_cast<uint64_t> (1) << 52;
        std::filesystem::path sourcePath("../HRIR/KEMAR/elev0/L0e");
        if(i < 100)
            sourcePath += "0";
        if(i < 10)
            sourcePath += "0";

        sourcePath += std::to_string(i) + "a.wav";
        LerpFromBuffer(0, reinterpret_cast<float*>(sounds[curId].data), reinterpret_cast<float*>(sounds[nextId].data), sourcePath, encoder);

        //--------------------------------------------
        // Right Ear
        //--------------------------------------------

        curId = i << 32;
        //Evelavation is 0 for now
        curId |= static_cast<uint64_t>(1) << 52;
        curId |= static_cast<uint64_t>(1) << 51;

        nextId = ((i + 5 < 360 ? i + 5 : 0)) << 32;
        nextId |= static_cast<uint64_t> (1) << 52;
        nextId |= static_cast<uint64_t>(1) << 51;
        sourcePath = ("../HRIR/KEMAR/elev0/R0e");
        if(i < 100)
            sourcePath += "0";
        if(i < 10)
            sourcePath += "0";

        sourcePath += std::to_string(i) + "a.wav";
        LerpFromBuffer(1, reinterpret_cast<float*>(sounds[curId].data), reinterpret_cast<float*>(sounds[nextId].data), sourcePath, encoder);
    }
}

static void CreateKEMARAudioPack()
{
    PackageEncoder encoder;
    std::filesystem::directory_iterator top("../HRIR/KEMAR/");

    if(std::filesystem::exists("TestFiles/TESTKEMARHRIR.pck"))
        return;

    for(auto& elev : top)
    {
        if(elev.is_directory())
        {
            std::string elevPath(elev.path().string());
            if(elevPath.rfind("elev") == std::string::npos)
                continue;

            int elevLevel = stoi(elevPath.substr(elevPath.rfind("elev") + 4));
            if(elevLevel < 0)
                elevLevel += 360;

            std::filesystem::directory_iterator hrirs(elev.path());
            for(auto& hrir : hrirs)
            {
                if(hrir.path().extension() == ".dat")
                {
                    std::string hrirName(hrir.path().filename().string());

                    int angleStart = hrirName.find("e") + 1;
                    uint64_t angle = std::stoi(hrirName.substr(angleStart,angleStart+3));
                    uint64_t isRight = hrirName.find("L") != std::string::npos ? 0 : 1;

                    // All meta data ids are above 32 bits
                    // All user ids are below 32 bits

                    uint64_t id = angle << 32;
                    id |= static_cast<uint64_t>(elevLevel) << 41;

                    id |= static_cast<uint64_t>(1) << 52; // KEMAR audio data;

                    id |= static_cast<uint64_t>(isRight) << 51;

                    encoder.AddFile(WriteToWav(hrir.path()), id, Encoding::PCM);
                }
            }
        }
    }

    encoder.WritePackage("TestFiles/TESTKEMARHRIR.pck");
    LerpMissingHRIR(encoder, "TestFiles/TESTKEMARHRIR.pck");
    encoder.WritePackage("TestFiles/TESTKEMARHRIR.pck");
}

static void CreatIR()
{
    WavFile wav("TestFiles/level.wav");
    std::fstream tesConvert("TestFiles/TESTLIR1.wav", std::ios_base::binary | std::ios_base::out);
    RiffHeader riffHeader{{'R', 'I', 'F', 'F'},
                          0,
                          {'W', 'A', 'V', 'E'}};
    tesConvert.write(reinterpret_cast<char *>(&riffHeader), sizeof(riffHeader));
    FormatHeader fmt = wav.GetFormat();
    fmt.channel_count = 1;
    tesConvert.write(reinterpret_cast<const char *>(&fmt), sizeof(FormatHeader));
    GenericHeaderChunk dataChunk{{'d', 'a', 't', 'a'}, wav.GetDataSize()};
    dataChunk.chunkSize = 1024;
    tesConvert.write(reinterpret_cast<char *>(&dataChunk), sizeof(dataChunk));

    std::fstream tesConver2("TestFiles/TESTLIR2.wav", std::ios_base::binary | std::ios_base::out);

    tesConver2.write(reinterpret_cast<char *>(&riffHeader), sizeof(riffHeader));
    tesConver2.write(reinterpret_cast<const char *>(&fmt), sizeof(FormatHeader));
    tesConver2.write(reinterpret_cast<char *>(&dataChunk), sizeof(dataChunk));


    std::fstream readFrom("../HRIR/KEMAR/elev10/L10e110a.dat", std::ios_base::binary | std::ios_base::in);
    std::fstream readFrom2("../HRIR/KEMAR/elev10/R10e110a.dat", std::ios_base::binary | std::ios_base::in);
    ASSERT_TRUE(readFrom);
    ASSERT_TRUE(readFrom2);

    short buffer[512];
    readFrom.read(reinterpret_cast<char*>(buffer), 1024);

    short buffer2[512];
    readFrom2.read(reinterpret_cast<char*>(buffer2), 1024);

    for(int i = 0; i < 512; i++)
    {
        short v = (buffer[i] >> 8) | (buffer[i] << 8);
        tesConvert.write(reinterpret_cast<char*>(&v), sizeof(short));

         short v2 = (buffer2[i] >> 8) | (buffer2[i] << 8);
        tesConver2.write(reinterpret_cast<char*>(&v2), sizeof(short));
    }
}

//TEST(BuiltGameAudio, GameAudio)
//{
//    BuildPackageAllOpus(0, "SongPack.pck",
//                        "SFX/Antigua.wav",
//                        "SFX/Arp.wav",
//                        "SFX/Bass.wav",
//                        "SFX/Drums 1.wav",
//                        "SFX/Drums 2.wav",
//                        "SFX/Drums 3.wav",
//                        "SFX/Handshake Firm Credits.wav");
//
//    BuildPackageAllPCM(100, "SFXPack.pck",
//                       "SFX/Crouch.wav",
//                       "SFX/DownAttackLand.wav",
//                       "SFX/Jump_1.wav",
//                       "SFX/LogoGlitch1.wav",
//                       "SFX/Player_Death.wav",
//                       "SFX/Player_Hurt.wav",
//                       "SFX/Player_Land.wav",
//                       "SFX/Shuriken_Impact_1.wav",
//                       "SFX/Shuriken_Impact_Damage.wav",
//                       "SFX/Single_Shuriken_Throw.wav",
//                       "SFX/SpawnEffect1.wav",
//                       "SFX/Step1.wav",
//                       "SFX/Sword_Deflect1.wav",
//                       "SFX/Sword_Slash1.wav",
//                       "SFX/SwordStick_Wall.wav",
//                       "SFX/Sword_Throw1.wav",
//                       "SFX/User Interface, Digital, Glitch 04 SND44936.wav",
//                       "SFX/User Interface, Digital, Glitch 04 SND44936 1.wav",
//                       "SFX/Whoosh.wav");
//}

TEST(HRTF, HRTFAtOnePoint)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    HRIRCalculator<float> hrir(packageManager);
    //hrir.SetAngle(110);
    //hrir.SetElev(10);


    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {0.312,0,0.45});
    Transform trans;
    trans.forward = {0,0,1};
    objectManager.SetListenerTransform(trans);

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, convolver);
    simulateEventManager(eventManager, "TestFiles/TESTConvoler.wav", 512);
    //SumAllInPackageWithFFT("TestFiles/TESTConvBank.pck", "TestFiles/TESTConvFFT.wav", 1024);
}

TEST(GameObject, LeftOfListener)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    HRIRCalculator<float> hrir(packageManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {-1,0,0});
    Transform trans;
    trans.forward = {0,0,1};
    objectManager.SetListenerTransform(trans);

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, convolver);
    simulateEventManager(eventManager, "TestFiles/TESTGameobjectLeft.wav", 512);
}

TEST(GameObject, RightOfListener)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {1,0,0});
    Transform trans;
    trans.forward = {0,0,1};
    objectManager.SetListenerTransform(trans);

    HRIRCalculator<float> hrir(packageManager);

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, convolver);
    simulateEventManager(eventManager, "TestFiles/TESTGameobjectRight.wav", 512);
}

TEST(GameObject, FrontOfListener)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {0,0,1});
    Transform trans;
    trans.forward = {0,0,1};
    objectManager.SetListenerTransform(trans);

    HRIRCalculator<float> hrir(packageManager);

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, convolver);
    simulateEventManager(eventManager, "TestFiles/TESTGameobjectFront.wav", 512);
}

TEST(GameObject, BehinedOfListener)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {0,0,-1});
    Transform trans;
    trans.forward = {0,0,1};
    objectManager.SetListenerTransform(trans);

    HRIRCalculator<float> hrir(packageManager);

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, convolver);
    simulateEventManager(eventManager, "TestFiles/TESTGameobjectBack.wav", 512);
}

TEST(HRTF, HRTFRotationSlow)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {10,0,10});

    HRIRCalculator<float> hrir(packageManager);
    //hrir.SetAngle(110);
    //hrir.SetElev(10);


    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, convolver);
    simulateEventManagerWithCalulator(eventManager, "TestFiles/TESTConvolerRotatingSlow.wav", 512, 10, 10, objectManager, 1/4.0f);
}

TEST(HRTF, HRTFRotationFast)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {10,0,10});

    HRIRCalculator<float> hrir(packageManager);
    //hrir.SetAngle(110);
    //hrir.SetElev(10);


    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, convolver);
    simulateEventManagerWithCalulator(eventManager, "TestFiles/TESTConvolerRotatingFast.wav", 512, 10, 10, objectManager, 1);
}

//TEST(HRTF, HRTFElevation)
//{
//
//    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");
//
//    PackageManager packageManager;
//    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
//    packageManager.LoadPack("../HRIR/KEMAR_s.pck");
//
//    GameObjectManager objectManager;
//    EventManager eventManager(packageManager,objectManager);
//    objectManager.AddObject(10);
//    objectManager.SetGameObjectPosition(10, {10, -10, 0});
//    HRIRCalculator<float> hrir(packageManager);
//    //hrir.SetAngle(110);
//    //hrir.SetElev(10);
//
//
//    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);
//
//    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);
//
//    eventManager.AddEvent(10, sample, convolver);
//    simulateEventManagerWithElevation(eventManager, "TestFiles/TESTConvolerElevation.wav", 512, 10, 10, objectManager, 1);
//}

TEST(ITD, ITDRotationFast)
{

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);

    ITD* itd = new ITD();

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, itd);
    simulateEventManagerWithCalulator(eventManager, "TestFiles/TESTITDRotatingFast.wav", 512, 10, 10, objectManager, 1);
}

TEST(ITD, ITDRotationSlow)
{

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);

    ITD* itd = new ITD();

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, itd);
    simulateEventManagerWithCalulator(eventManager, "TestFiles/TESTITDRotatingSlow.wav", 512, 10, 10, objectManager, 1/4.0f);
}

TEST(ITD, ITDAtOnePoint)
{

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);

    objectManager.SetGameObjectPosition(10, {0,50,50});

    ITD* itd = new ITD();

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, itd);
    simulateEventManager(eventManager, "TestFiles/TESTITDPoint.wav", 512);
}


TEST(ITD, ITDDopplerForward)
{

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/Siren.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {1,0,-50});
    ITD* itd = new ITD();

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, itd);
    simulateEventManagerWithDirection(eventManager, "TestFiles/TESTITDDopperForward.wav", 512, 10,  objectManager, {0,0,1/4.0f});
}


TEST(ITD, ITDDoppleLeftRight)
{

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/Siren.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {-50,0,1});
    ITD* itd = new ITD();

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, itd);
    simulateEventManagerWithDirection(eventManager, "TestFiles/TESTITDDopplerLeftRight.wav", 512, 10,  objectManager, {1/4.0f,0,0});
}


TEST(Audio3D, RotationFast)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, IVector3{10,0,0});

    HRIRCalculator<float> hrir(packageManager);
    ITD* itd = new ITD();

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    FirstOrderLowpass<float>* lowpass = new FirstOrderLowpass<float>;
    DistanceAttenuation* attenuation = new DistanceAttenuation(lowpass);

    eventManager.AddEvent(10, sample,attenuation, convolver, itd);
    simulateEventManagerWithCalulator(eventManager, "TestFiles/TEST3DAudio.wav", 512, 10, 10, objectManager, 1);
}


TEST(Audio3D, DoppleLeftRight)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/Siren.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {20,0,-130});

    FirstOrderLowpass<float>* pass = new FirstOrderLowpass<float>;
    DistanceAttenuation* da = new DistanceAttenuation(pass);

    HRIRCalculator<float> hrir(packageManager);
    ITD* itd = new ITD();

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    GameObject::SetParam("RolloffFunc", 2.0f);

    eventManager.AddEvent(10, sample, da, convolver, itd);
    simulateEventManagerWithDirection(eventManager, "TestFiles/TEST3DAudioDopplerRightLeft.wav", 512, 10,  objectManager, {0,0,1/4.0f});
}

TEST(Audio3D, DoppleForward)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/Siren.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {-130,0,20});

    FirstOrderLowpass<float>* pass = new FirstOrderLowpass<float>;
    DistanceAttenuation* da = new DistanceAttenuation(pass);

    HRIRCalculator<float> hrir(packageManager);
    ITD* itd = new ITD();

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    GameObject::SetParam("RolloffFunc", 1.0f);

    eventManager.AddEvent(10, sample, da, convolver, itd);
    simulateEventManagerWithDirection(eventManager, "TestFiles/TEST3DAudioDopplerForward.wav", 512, 10,  objectManager, {1/4.0f,0,0});
}


TEST(Filters, WavLoop)
{
    BuildPackageAllPCM(0, "TestFiles/TESTWavBank.pck","TestFiles/Slash2.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    sample->SetLoopCount(10);

    eventManager.AddEvent(sample);
    simulateEventManager(eventManager, "TestFiles/TESTWavLooping.wav", 512);
}

TEST(Filters, WavLoopAndShift)
{
    BuildPackageAllPCM(0, "TestFiles/TESTWavBank.pck","TestFiles/Slash2.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    sample->SetLoopCount(10);
    sample->SetRandomPitchRange(-1200, 1200);

    eventManager.AddEvent(sample);
    simulateEventManager(eventManager, "TestFiles/TESTWavLoopShift.wav", 512);
}

TEST(Filters, OpusLoop)
{
    BuildPackageAllOpus(0, "TestFiles/TESTWavBank.pck","TestFiles/Slash2.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    OpusContainer<float>* sample = new OpusContainer<float>(packageManager.GetSounds()[0]);

    sample->SetLoopCount(10);

    eventManager.AddEvent(sample);
    simulateEventManager(eventManager, "TestFiles/TESTOpusLooping.wav", 512);
}

TEST(Filters, OpusLoopAndShift)
{
    BuildPackageAllOpus(0, "TestFiles/TESTWavBank.pck","TestFiles/Slash2.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    OpusContainer<float>* sample = new OpusContainer<float>(packageManager.GetSounds()[0]);

    sample->SetLoopCount(10);
    sample->SetRandomPitchRange(-1200, 1200);

    eventManager.AddEvent(sample);
    simulateEventManager(eventManager, "TestFiles/TESTOpusLoopShift.wav", 512);
}

TEST(Filters, FFTTest)
{
    pffft::Fft<float> fft(32);
    float vec[32] = {0};
    float vec2[32] = {0};
    vec[0] = 1;
    std::complex<float> out[32] = {};
    fft.forward(vec, out);

    fft.inverse(out, vec2);

    ASSERT_EQ(vec[0], vec2[0]/32);
}

TEST(Filters, FirstOrderLowpassMoving)
{
    BuildPackageAllPCM(0, "TestFiles/TESTWavBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    FirstOrderLowpass<float>* lowpass1 = new FirstOrderLowpass<float>;
    FirstOrderLowpass<float>* lowpass2 = new FirstOrderLowpass<float>;

    DualBiquad<float>* qaud = new DualBiquad<float>(lowpass1, lowpass2);

    eventManager.AddEvent(sample, qaud);
    simulateEventManagerDecreasingCutoff(eventManager, "TestFiles/TESTFirstOrderLowPassMoving.wav", 512, qaud);
}

TEST(Filters, FirstOrderLowpassStill)
{
    BuildPackageAllPCM(0, "TestFiles/TESTWavBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    FirstOrderLowpass<float>* lowpass1 = new FirstOrderLowpass<float>;
    FirstOrderLowpass<float>* lowpass2 = new FirstOrderLowpass<float>;

    DualBiquad<float>* quad = new DualBiquad<float>(lowpass1, lowpass2);
    quad->SetCutoff(1000);

    eventManager.AddEvent(sample, quad);
    simulateEventManager(eventManager, "TestFiles/TESTFirstOrderLowPassStill.wav", 512);
}

TEST(Filters, SecondOrderLowpassMoving)
{
    BuildPackageAllPCM(0, "TestFiles/TESTWavBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    SecondOrderLowpass<float>* lowpass1 = new SecondOrderLowpass<float>;
    SecondOrderLowpass<float>* lowpass2 = new SecondOrderLowpass<float>;

    DualBiquad<float>* qaud = new DualBiquad<float>(lowpass1, lowpass2);

    eventManager.AddEvent(sample, qaud);
    simulateEventManagerDecreasingCutoff(eventManager, "TestFiles/TESTSecondOrderLowPassMoving.wav", 512, qaud);
}

TEST(Filters, SecondOrderLowpassStill)
{
    BuildPackageAllPCM(0, "TestFiles/TESTWavBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    SecondOrderLowpass<float>* lowpass1 = new SecondOrderLowpass<float>;
    SecondOrderLowpass<float>* lowpass2 = new SecondOrderLowpass<float>;

    DualBiquad<float>* quad = new DualBiquad<float>(lowpass1, lowpass2);
    quad->SetCutoff(1000);

    eventManager.AddEvent(sample, quad);
    simulateEventManager(eventManager, "TestFiles/TESTSecondOrderLowPassStill.wav", 512);
}

TEST(Filters, LinkwitzRileyLowpassMoving)
{
    BuildPackageAllPCM(0, "TestFiles/TESTWavBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

   LinkwitzRileyLowpass<float>* lowpass1 = new LinkwitzRileyLowpass<float>;
   LinkwitzRileyLowpass<float>* lowpass2 = new LinkwitzRileyLowpass<float>;

    DualBiquad<float>* qaud = new DualBiquad<float>(lowpass1, lowpass2);

    eventManager.AddEvent(sample, qaud);
    simulateEventManagerDecreasingCutoff(eventManager, "TestFiles/TESTLinkwitzRileyLowpassMoving.wav", 512, qaud);
}

TEST(Filters, LinkwitzRileyLowpassStill)
{
    BuildPackageAllPCM(0, "TestFiles/TESTWavBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTWavBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager, objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    SecondOrderLowpass<float>* lowpass1 = new SecondOrderLowpass<float>;
    SecondOrderLowpass<float>* lowpass2 = new SecondOrderLowpass<float>;

    DualBiquad<float>* quad = new DualBiquad<float>(lowpass1, lowpass2);
    quad->SetCutoff(1000);

    eventManager.AddEvent(sample, quad);
    simulateEventManager(eventManager, "TestFiles/TESTLinkwitzRileyLowpassStill.wav", 512);
}

void DistanceHelper(char* outName, float value)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/Siren.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);
    objectManager.SetGameObjectPosition(10, {1,0,-130});
    FirstOrderLowpass<float>* pass = new FirstOrderLowpass<float>;
    DistanceAttenuation* itd = new DistanceAttenuation(pass);
    FirstOrderLowpass<float>* pass2 = new FirstOrderLowpass<float>;
    DistanceAttenuation* itd2 = new DistanceAttenuation(pass2);

    DualFilter* dualFilter = new DualFilter(itd, itd2);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    GameObject::SetParam("RolloffFunc", value);

    eventManager.AddEvent(10, sample, dualFilter);
    simulateEventManagerWithDirection(eventManager, outName, 512, 10,  objectManager, {0,0,1/4.0f});
}

TEST(DistanceAttenuation, ForwardFunc1)
{
    DistanceHelper("TestFiles/TESTDistanceForwardFunc1.wav", 0);
}
TEST(DistanceAttenuation, ForwardFunc2)
{
    DistanceHelper("TestFiles/TESTDistanceForwardFunc2.wav", 1);
}
TEST(DistanceAttenuation, ForwardFunc3)
{
    DistanceHelper("TestFiles/TESTDistanceForwardFunc3.wav", 2);
}
TEST(DistanceAttenuation, ForwardFunc4)
{
    DistanceHelper("TestFiles/TESTDistanceForwardFunc4.wav", 3);
}
TEST(DistanceAttenuation, ForwardFunc5)
{
    DistanceHelper("TestFiles/TESTDistanceForwardFunc5.wav", 4);
}
TEST(DistanceAttenuation, ForwardFunc6)
{
    DistanceHelper("TestFiles/TESTDistanceForwardFunc6.wav", 5);
}
TEST(DistanceAttenuation, ForwardFunc7)
{
    DistanceHelper("TestFiles/TESTDistanceForwardFunc7.wav", 6);
}

static void Get2048Samples(Frame<float>* samples)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");
    IO::MemoryMappedFile package("TestFiles/TESTConvBank.pck");
//    std::unordered_map<uint64_t, SoundData> data;
//    PackageDecoder::DecodePackage(data, package);
    PackageManager data;
    data.LoadPack("TestFiles/TESTConvBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(data,objectManager);

    unsigned largestSize = 0;

    Filter<float> *filter = nullptr;

    for (auto iter = data.GetSounds().begin(); iter != data.GetSounds().end(); ++iter)
    {
        largestSize = std::max(iter->second.sampleCount, largestSize);
        if (iter->second.audioType == Encoding::PCM)
        {
            filter = new WavContainer<float>(iter->second);

        } else
        {
            filter = new OpusContainer<float>(iter->second);
        }
        eventManager.AddEvent(filter);
    }

    for(int i = 0; i < 44100; i += 2048)
    {
        eventManager.GetSamplesFromAllEvents(2048, samples);
    }

}

static void PFFFT2048(benchmark::State& state)
{
    const int size = 2048;
    float data[size] = {};
    std::complex<float> complex[size] = {};
    Frame<float> songData[2048];
    Get2048Samples(songData);
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    pffft::Fft<float> fft(size);
    for(auto _ : state)
    {
        fft.forwardToInternalLayout(data, reinterpret_cast<float *>(complex));
        fft.forwardToInternalLayout(data, reinterpret_cast<float *>(complex));
        fft.convolve(reinterpret_cast<const float *>(complex), reinterpret_cast<const float *>(complex), data, 1.0f/512);
        fft.inverseFromInternalLayout(data, reinterpret_cast<float *>(songData));
    }
}
BENCHMARK(PFFFT2048);

static void PFFFT1024(benchmark::State& state)
{
    const int size = 1024;
    float data[2048] = {};
    std::complex<float> complex[size] = {};
    Frame<float> songData[2048];
    Get2048Samples(songData);
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    pffft::Fft<float> fft(size);
    for(auto _ : state)
    {
        fft.forwardToInternalLayout(data, reinterpret_cast<float *>(complex));
        fft.forwardToInternalLayout(data, reinterpret_cast<float *>(complex));
        fft.convolve(reinterpret_cast<const float *>(complex), reinterpret_cast<const float *>(complex), data, 1.0f/512);
        fft.inverseFromInternalLayout(data, reinterpret_cast<float *>(songData));
    }
}
BENCHMARK(PFFFT1024);

static void PFFFT512(benchmark::State& state)
{
    const int size = 512;
    float data[2048] = {};
    std::complex<float> complex[size] = {};
    Frame<float> songData[2048];
    Get2048Samples(songData);
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    pffft::Fft<float> fft(size);
    for(auto _ : state)
    {
        fft.forwardToInternalLayout(data, reinterpret_cast<float *>(complex));
        fft.forwardToInternalLayout(data, reinterpret_cast<float *>(complex));
        fft.convolve(reinterpret_cast<const float *>(complex), reinterpret_cast<const float *>(complex), data, 1.0f/512);
        fft.inverseFromInternalLayout(data, reinterpret_cast<float *>(songData));
    }
}
BENCHMARK(PFFFT512);


#endif //I_SOUND_ENGINE_FILTERMODULE_H

static void Add1GameObjects(benchmark::State& state)
{
    for(auto _ : state)
    {
        GameObjectManager objectManager;
        objectManager.AddObject(10);
    }
}
BENCHMARK(Add1GameObjects);

static void Add100GameObjects(benchmark::State& state)
{
    for(auto _ : state)
    {
        GameObjectManager objectManager;
        for(int i = 1; i < 101; ++i)
        {
            objectManager.AddObject(i);
        }
    }
}
BENCHMARK(Add100GameObjects);

static void Update1GameObjects(benchmark::State& state)
{
    GameObjectManager objectManager;
    objectManager.AddObject(10);
    for(auto _ : state)
    {
        objectManager.SetGameObjectPosition(10, {1,0,0});
    }
}
BENCHMARK(Update1GameObjects);

static void Update100GameObjects(benchmark::State& state)
{
    GameObjectManager objectManager;
    for(int i = 1; i < 101; ++i)
    {
        objectManager.AddObject(i);
    }
    for(auto _ : state)
    {
        for(int i = 1; i < 101; ++i)
        {
            objectManager.SetGameObjectPosition(i, {1,0,0});
        }
    }
}
BENCHMARK(Update100GameObjects);

static void Get1GameObjects(benchmark::State& state)
{
    GameObjectManager objectManager;
    objectManager.AddObject(10);
    for(auto _ : state)
    {
        GameObject obj;
        objectManager.GetGameObject(10, obj);
        benchmark::DoNotOptimize(obj);
    }
}
BENCHMARK(Get1GameObjects);

static void Get100GameObjects(benchmark::State& state)
{
    GameObjectManager objectManager;
    for(int i = 1; i < 101; ++i)
    {
        objectManager.AddObject(i);
    }
    for(auto _ : state)
    {
        for(int i = 1; i < 101; ++i)
        {
            GameObject obj;
            objectManager.GetGameObject(i, obj);
            benchmark::DoNotOptimize(obj);
        }
    }
}
BENCHMARK(Get100GameObjects);


static void ReadMono512Samples(benchmark::State& state)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    Event event(0);
    event.AddFilter(sample);

    //eventManager.AddEvent(sample, convolver);
    Frame<float> buff[512];

    GameObject obj;
    event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);

    for(auto _ : state)
    {

        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
        sample->Reset();
    }
}
BENCHMARK(ReadMono512Samples);

static void ReadStereo512Samples(benchmark::State& state)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    Event event(0);
    event.AddFilter(sample);

    //eventManager.AddEvent(sample, convolver);
    Frame<float> buff[512];

    GameObject obj;
    event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);

    for(auto _ : state)
    {

        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
        sample->Reset();
    }
}
BENCHMARK(ReadStereo512Samples);

static void FirstOrderLowpass512Samples(benchmark::State& state)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    FirstOrderLowpass<float>* lowpass = new FirstOrderLowpass<float>;
    lowpass->SetCutoff(1000);

    Event event(0);
    event.AddFilter(sample);
    event.AddFilter(lowpass);

    //eventManager.AddEvent(sample, convolver);
    Frame<float> buff[512];

    GameObject obj;
    event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);

    for(auto _ : state)
    {
        //lowpass->GetNextSamples(512, &buff->leftChannel, &buff->leftChannel, obj);
        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
        sample->Reset();
    }
}
BENCHMARK(FirstOrderLowpass512Samples);

static void SecondOrderLowpass512Samples(benchmark::State& state)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    SecondOrderLowpass<float>* lowpass = new SecondOrderLowpass<float>;
    lowpass->SetCutoff(1000);

    Event event(0);
    event.AddFilter(sample);
    event.AddFilter(lowpass);

    //eventManager.AddEvent(sample, convolver);
    Frame<float> buff[512];

    GameObject obj;
    event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);

    for(auto _ : state)
    {
        //lowpass->GetNextSamples(512, &buff->leftChannel, &buff->leftChannel, obj);
        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
        sample->Reset();
    }
}
BENCHMARK(SecondOrderLowpass512Samples);

static void LinkwitzRielyLowpass512Samples(benchmark::State& state)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    LinkwitzRileyLowpass<float>* lowpass = new LinkwitzRileyLowpass<float>;
    lowpass->SetCutoff(1000);

    Event event(0);
    event.AddFilter(sample);
    event.AddFilter(lowpass);

    //eventManager.AddEvent(sample, convolver);
    Frame<float> buff[512];

    GameObject obj;
    event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);

    for(auto _ : state)
    {
        //lowpass->GetNextSamples(512, &buff->leftChannel, &buff->leftChannel, obj);
        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
        sample->Reset();
    }
}
BENCHMARK(LinkwitzRielyLowpass512Samples);

static void DistanceAttenuation512Samples(benchmark::State& state)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    FirstOrderLowpass<float>* lowpass = new FirstOrderLowpass<float>;

    DistanceAttenuation* attenuation = new DistanceAttenuation(lowpass);

    Event event(0);
    event.AddFilter(sample);
    event.AddFilter(attenuation);

    //eventManager.AddEvent(sample, convolver);
    Frame<float> buff[512];

    GameObject obj;
    event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);

    for(auto _ : state)
    {
        //lowpass->GetNextSamples(512, &buff->leftChannel, &buff->leftChannel, obj);
        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
        sample->Reset();
    }
}
BENCHMARK(DistanceAttenuation512Samples);

static void HRTF512Samples(benchmark::State& state)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);

    HRIRCalculator<float> hrir(packageManager);

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    Event event(0);
    event.AddFilter(sample);
    event.AddFilter(convolver);

    //eventManager.AddEvent(sample, convolver);
    Frame<float> buff[512];

    GameObject obj;
    event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);

    for(auto _ : state)
    {
        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
        sample->Reset();
    }
}
BENCHMARK(HRTF512Samples);

static void ITD512Samples(benchmark::State& state)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);

    ITD* itd = new ITD();

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    Event event(0);
    event.AddFilter(sample);
    event.AddFilter(itd);

    //eventManager.AddEvent(sample, convolver);
    Frame<float> buff[512];

    GameObject obj;

    event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
    for(auto _ : state)
    {
        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
        sample->Reset();
    }
}
BENCHMARK(ITD512Samples);


static void Combined512Samples(benchmark::State& state)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");
    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);

    HRIRCalculator<float> hrir(packageManager);

    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    ITD* itd = new ITD();

    FirstOrderLowpass<float>* pass2 = new FirstOrderLowpass<float>;
    DistanceAttenuation* itd2 = new DistanceAttenuation(pass2);

    Event event(0);
    event.AddFilter(sample);
    event.AddFilter(itd2);
    event.AddFilter(convolver);
    event.AddFilter(itd);

    //eventManager.AddEvent(sample, convolver);
    Frame<float> buff[512];

    GameObject obj;
    event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);

    for(auto _ : state)
    {
        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
        sample->Reset();
    }
}
BENCHMARK(Combined512Samples);

//TEST(buildBank, testBank)
//{
//    BuildPackageAllPCM(0, "TestFiles/TESTEventPack.pck", "TestFiles/DrySignal.wav", "TestFiles/level.wav", "TestFiles/Siren.wav");
//}