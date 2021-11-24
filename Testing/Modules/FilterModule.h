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

#include <filesystem>

#include <immintrin.h>

#include <bitset>

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

static void simulateEventManagerWithCalulator(EventManager& eventManager, const char* outFileName, int frameSize, int id, int jump, GameObjectManager& objManager)
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
    int angle = 0;
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
            angle += 1;
            if (angle >= 360)
                angle -= 360;

            IVector3 newPos{std::cos(angle * 3.145f / 180.0f), 0,std::sin(angle * 3.145f / 180.0f)};
            objManager.SetGameObjectPosition(id, newPos);
        }

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

    for(int i = 0; i < 512; i++)
    {
        short v = (buffer[i] >> 8) | (buffer[i] << 8);
        tesConvert.write(reinterpret_cast<char*>(&v), sizeof(short));
    }
    return outFile;
}

static void CreateKEMARAudioPack()
{
    PackageEncoder encoder;
    std::filesystem::directory_iterator top("../HRIR/KEMAR/");

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


    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(sample, convolver);
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

TEST(HRTF, HRTFRotation)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

    PackageManager packageManager;
    packageManager.LoadPack("TestFiles/TESTConvBank.pck");
    packageManager.LoadPack("TestFiles/TESTKEMARHRIR.pck");

    GameObjectManager objectManager;
    EventManager eventManager(packageManager,objectManager);
    objectManager.AddObject(10);

    HRIRCalculator<float> hrir(packageManager);
    //hrir.SetAngle(110);
    //hrir.SetElev(10);


    ConvolutionFreq* convolver = new ConvolutionFreq(512, hrir);

    WavContainer<float>* sample = new WavContainer<float>(packageManager.GetSounds()[0]);

    eventManager.AddEvent(10, sample, convolver);
    simulateEventManagerWithCalulator(eventManager, "TestFiles/TESTConvolerRotating.wav", 512, 10, 10, objectManager);
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
        fft.forward(data, complex);
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
        fft.forward(data, complex);

        fft.forward(data, complex);

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
        fft.forward(data, complex);


        fft.forward(data+512, complex);


        fft.forward(data+1024, complex);


        fft.forward(data + 1536, complex);
    }
}
BENCHMARK(PFFFT512);

static void HRTF512Samples(benchmark::State& state)
{
    CreateKEMARAudioPack();

    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/DrySignal.wav");

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
    for(auto _ : state)
    {
        event.GetSamples(512, &buff->leftChannel, &buff[256].leftChannel, obj);
    }
}
BENCHMARK(HRTF512Samples);

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