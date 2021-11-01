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

#include <immintrin.h>

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

static void Get2048Samples(Frame<float>* samples)
{
    BuildPackageAllPCM(0, "TestFiles/TESTConvBank.pck","TestFiles/level.wav");
    IO::MemoryMappedFile package("TestFiles/TESTConvBank.pck");
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


constexpr double PI = 3.1415926535897932,
        DELTA = 0.00003051757; // 2^-15
typedef std::complex<double>* complexList;

std::array<std::array<std::complex<double>,1024>,11> lookupTable = []
{
    std::array<std::array<std::complex<double>,1024>,11> arr = {};
    for(int s = 1; s <= 11; ++s)
    {
        int m = 1 << s;
        std::complex<double> wm(std::cos((2.0f * PI) / m ), -std::sin((2.0f * PI) / m));
        std::complex<double> w = 1;
        for(int j = 0; j < (m/2.0f); ++j)
        {
            arr[s-1][j] = w;
            w *= wm;

        }
        arr[10][1023] = w;
    }
    return arr;
}();

static void bitReverseOrder(double* list, complexList& newList, unsigned n)
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


template<typename Type>
static void fft2(double* list, int n, complexList& rList)
{
    bitReverseOrder(list, rList, n);

    for(int s = 1; s <= std::log2(n); ++s)
    {
        int m = 1 << s;

        for(int k = 0; k < n; k += m)
        {
            std::complex<Type> w = 1;

            for(int j = 0; j < (m/2.0f); ++j)
            {

                std::complex<Type> u = rList[k + j];
                __m256d uv = {u.real(), u.imag(), u.real(), u.imag()};

                __m256d wv = {w.real(), w.imag(), w.imag(), w.real()};

                std::complex<Type> t = lookupTable[s-1][j] * rList[k + j + m/2];;
                __m256d tv = {t.real(), t.imag(), -t.real(), -t.imag()};

                __m256d r = _mm256_add_pd(tv, uv);

                rList[k + j] = *reinterpret_cast<std::complex<double>*>(&r);
                rList[k + j + m/2] = *reinterpret_cast<std::complex<double>*>(&r);

//                rList[k + j] = u + t;
//                rList[k + j + m/2] = u - t;

                w = lookupTable[s-1][j];
            }
        }
    }
}

static void MyFFT2048(benchmark::State& state)
{
    const int size = 2048;
    complexList complex = new std::complex<double>[size];
    double data[2048] = {};
    Frame<float> songData[2048];
    Get2048Samples(songData);
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }

    for(auto _ : state)
    {
        fft2<double>(data, size, complex);
    }
}
BENCHMARK(MyFFT2048);

static void MyFFT1024(benchmark::State& state)
{
    const int size = 1024;
    complexList complex = new std::complex<double>[size];
    double data[2048] = {};
    Frame<float> songData[2048];
    Get2048Samples(songData);
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    for(auto _ : state)
    {
        fft2<double>(data, size, complex);
        fft2<double>(data, size, complex);
    }
}
BENCHMARK(MyFFT1024);

static void MyFFT512(benchmark::State& state)
{
    const int size = 512;
    complexList complex = new std::complex<double>[size];
    double data[2048] = {};
    Frame<float> songData[2048];
    Get2048Samples(songData);
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }
    for(int i = 0; i < 2048; ++i)
    {
        data[i] = songData[i].leftChannel;
    }

    for(auto _ : state)
    {
        fft2<double>(data, size, complex);
        fft2<double>(data, size, complex);
        fft2<double>(data, size, complex);
        fft2<double>(data, size, complex);
    }
}
BENCHMARK(MyFFT512);





#endif //I_SOUND_ENGINE_FILTERMODULE_H
