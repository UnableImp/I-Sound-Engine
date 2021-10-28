//
// Created by zack on 10/27/21.
//

#ifndef I_SOUND_ENGINE_FILTERMODULE_H
#define I_SOUND_ENGINE_FILTERMODULE_H

#include "gtest/gtest.h"
#include "benchmark/benchmark.h"
#include "Events/Event.h"
#include "Events/EventManager.h"

#include "AudioFormats/WavFile.h"
#include "AudioPackage/PackageEncoder.h"
#include "AudioPackage/PackageDecoder.h"
#include "Filters/WavContainer.h"
#include "Filters/OpusContainer.h"
#include "Filters/ConvolutionFreq.h"

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

    Frame<float>* frame = new Frame<float>[frameSize];
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

static void SumAllInPackageWithFFT(const char* packageName, const char* outFileName, int frameSize)
{
    IO::MemoryMappedFile package(packageName);
    std::unordered_map<uint64_t, SoundData> data;
    PackageDecoder::DecodePackage(data, package);

    EventManager eventManager(data);

    unsigned largestSize = 0;

    Filter<float> *filter = nullptr;

    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
        largestSize = std::max(iter->second.sampleCount, largestSize);
        if (iter->second.audioType == Encoding::PCM)
        {
            filter = new WavContainer<float>(iter->second);

        } else
        {
            filter = new OpusContainer<float>(iter->second);

        }
        eventManager.AddEvent(filter, new ConvolutionFreq(frameSize));
    }

    simulateEventManager(eventManager, outFileName, frameSize);
}

TEST(Filters, ConvolutionFreqFFTOnly)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");
    SumAllInPackageWithFFT("TestFiles/TESTConvBank.pck", "TestFiles/TESTConvFFT.wav", 1024);
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

static void PFFFT2048(benchmark::State& state)
{
    const int size = 2048;
    float data[size] = {};
    std::complex<float> complex[size] = {};
    data[0] = 1;
    pffft::Fft<float> fft(size);
    for(auto _ : state)
    {
        fft.forward(data, complex);
//        fft.inverse(complex, data);
//        for(int i = 0; i < size; ++i)
//        {
//            data[i] /= size;
//        }
    }
}
BENCHMARK(PFFFT2048);

static void PFFFT1024(benchmark::State& state)
{
    const int size = 1024;
    float data[size] = {};
    std::complex<float> complex[size] = {};
    data[0] = 1;
    pffft::Fft<float> fft(size);
    for(auto _ : state)
    {
        fft.forward(data, complex);
//        fft.inverse(complex, data);
//        for(int i = 0; i < size; ++i)
//        {
//            data[i] /= size;
//        }
        fft.forward(data, complex);
//        fft.inverse(complex, data);
//        for(int i = 0; i < size; ++i)
//        {
//            data[i] /= size;
//        }
    }
}
BENCHMARK(PFFFT1024);

static void PFFFT512(benchmark::State& state)
{
    const int size = 512;
    float data[size] = {};
    std::complex<float> complex[size] = {};
    data[0] = 1;
    pffft::Fft<float> fft(size);
    for(auto _ : state)
    {
        fft.forward(data, complex);
//        fft.inverse(complex, data);
//        for(int i = 0; i < size; ++i)
//        {
//            data[i] /= size;
//        }

        fft.forward(data, complex);
//        fft.inverse(complex, data);
//        for(int i = 0; i < size; ++i)
//        {
//            data[i] /= size;
//        }

        fft.forward(data, complex);
//        fft.inverse(complex, data);
//        for(int i = 0; i < size; ++i)
//        {
//            data[i] /= size;
//        }

        fft.forward(data, complex);
//        fft.inverse(complex, data);
//        for(int i = 0; i < size; ++i)
//        {
//            data[i] /= size;
//        }
    }
}
BENCHMARK(PFFFT512);


constexpr double PI = 3.1415926535897932,
        DELTA = 0.00003051757; // 2^-15
typedef std::vector<std::complex<double>> complexList;

static void bitReverseOrder(complexList const& list, complexList& newList, unsigned n)
{
    int size = std::log2(n - 1) + 1;
    int sum = 0;
    newList[0] = list[0];

    for(unsigned i = 1; i < n; ++i)
    {
        int shift = size - 1;
        while(sum & 1 << shift)
        {
            sum ^= 1 << shift;
            --shift;
        }
        sum |= 1 << shift;

        newList[sum] = list[i];
    }
}



static void fft2(complexList const& list, int n, complexList& rList)
{

    bitReverseOrder(list, rList, n);

    for(int s = 1; s <= std::log2(n); ++s)
    {
        int m = 1 << s;
        std::complex<double> wm(std::cos((2.0f * PI) / m ), -std::sin((2.0f * PI) / m));

        for(int k = 0; k < n; k += m)
        {
            std::complex<double> w = 1;

            for(int j = 0; j < (m/2.0); ++j)
            {
                std::complex<double> t = w * rList[k + j + m/2];

                std::complex<double> u = rList[k + j];

                rList[k + j] = u + t;
                rList[k + j + m/2] = u - t;

                w *= wm;
            }
        }
    }
}

static void MyFFT2048(benchmark::State& state)
{
    const int size = 2048;
    complexList data(size);
    complexList complex(size);
    data[0] = 1;
    for(auto _ : state)
    {
        fft2(data, size, complex);
    }
}
BENCHMARK(MyFFT2048);

static void MyFFT1024(benchmark::State& state)
{
    const int size = 1024;
    complexList data(size);
    complexList complex(size);
    data[0] = 1;
    for(auto _ : state)
    {
        fft2(data, size, complex);
        fft2(data, size, complex);
    }
}
BENCHMARK(MyFFT1024);

static void MyFFT512(benchmark::State& state)
{
    const int size = 512;
    complexList data(size);
    complexList complex(size);
    data[0] = 1;
    for(auto _ : state)
    {
        fft2(data, size, complex);
        fft2(data, size, complex);
        fft2(data, size, complex);
        fft2(data, size, complex);
    }
}
BENCHMARK(MyFFT512);

#endif //I_SOUND_ENGINE_FILTERMODULE_H
