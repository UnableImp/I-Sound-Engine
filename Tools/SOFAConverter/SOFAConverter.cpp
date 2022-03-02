//
// Created by zacke on 2/28/2022.
//
#include <iostream>
#include "SOFAConverter.h"


#include "AudioFormats/WavFile.h"
#include "AudioPackage/PackageEncoder.h"
#include "Filters/WavContainer.h"

#include "mysofa.h"

#include <filesystem>

constexpr float delta = 0.003f;

static void WriteFileFromBuffer(const float* outData, std::string path, int count, float scaler )
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
    dataChunk.chunkSize = count * 2;
    tesConvert.write(reinterpret_cast<char *>(&dataChunk), sizeof(dataChunk));
    std::fstream tesConver2(outFile.c_str(), std::ios_base::binary | std::ios_base::out);

    tesConver2.write(reinterpret_cast<char *>(&riffHeader), sizeof(riffHeader));
    tesConver2.write(reinterpret_cast<const char *>(&fmt), sizeof(FormatHeader));
    tesConver2.write(reinterpret_cast<char *>(&dataChunk), sizeof(dataChunk));

    for(int i = 0; i < count; i++)
    {
        if(std::abs(outData[i] * scaler) > 1)
            std::cout << "problem: " << outData[i-1] << " " << outData[i] << " " << outData[i+1] <<  std::endl;
        short v = outData[i] * (1<<15) * scaler;

        tesConvert.write(reinterpret_cast<char*>(&v), sizeof(short));
    }
}


static void WriteFileFromBufferMinPhase(const float* outData, std::string path, int count, float scaler )
{
    for(int i = 0; i < count; ++i)
    {
        if(outData[i] > delta)
        {
            WriteFileFromBuffer(outData + i, path, count - i, scaler);
            break;
        }
    }
}



int SOFAToPck(char* sofaIn, char* packOut, uint64_t HRIRid)
{
    int filter_length;
    int err;
    struct MYSOFA_EASY* hrtf = nullptr;

    hrtf = mysofa_open_no_norm(sofaIn, 44100, &filter_length, &err);

    if(hrtf == nullptr)
    {
        std::cout << "Failed to open file: " << err << std::endl;
        return err;
    }

    float* leftIR = new float[filter_length];
    float* rightIR = new float[filter_length];
    float leftDelay;
    float rightDelay;

    std::filesystem::create_directory("toDelete/");

    PackageEncoder encoder;
    for(int j = -40; j < 90; ++j)
    {
        std::cout << j << std::endl;
        float z = j * 3.145f / 180.0f;

        std::string elv = "toDelete/";
        int encodingValue = j;
        if(encodingValue < 0)
        {
            encodingValue += 360;
        }

        if(encodingValue < 100)
            elv += "0";
        if(encodingValue < 10)
            elv += "0";
        elv += std::to_string(encodingValue);
        elv += "_";

        for (int i = 0; i < 360; ++i)
        {
            float x = std::cos(i * 3.145f / 180.0f);
            float y = std::sin(i * 3.145f / 180.0f);
            mysofa_getfilter_float_nointerp(hrtf, x, y, z, leftIR, rightIR, &leftDelay, &rightDelay);

            std::string path = elv;
            if (i < 100)
                path += "0";
            if (i < 10)
                path += "0";
            path += std::to_string(i);

            WriteFileFromBuffer(leftIR, path + "L.wav", filter_length, 0.25f);
            WriteFileFromBuffer(rightIR, path + "R.wav", filter_length, 0.25f);
            WriteFileFromBufferMinPhase(leftIR, path + "Lm.wav", filter_length, 0.25f);
            WriteFileFromBufferMinPhase(rightIR, path + "Rm.wav", filter_length, 0.25f);

            uint64_t id = static_cast<uint64_t >(i) << 32;
            id |= static_cast<uint64_t>(encodingValue) << 41;
            id |= static_cast<uint64_t>(HRIRid) << 52;
            if(encoder.AddFile(path + "L.wav", id, Encoding::PCM) != ErrorNum::NoErrors)
            {
                std::cout << "Failed to save file" << std::endl;
            }

            id |= static_cast<uint64_t>(1) << 51;
            if(encoder.AddFile(path + "R.wav", id, Encoding::PCM) != ErrorNum::NoErrors)
            {
                std::cout << "Failed to save file" << std::endl;
            }

            id = static_cast<uint64_t >(i) << 32;
            id |= static_cast<uint64_t>(encodingValue) << 41;
            id |= static_cast<uint64_t>(HRIRid) << 52;
            id |= static_cast<uint64_t>(1) << 55;
            if(encoder.AddFile(path + "Lm.wav", id, Encoding::PCM) != ErrorNum::NoErrors)
            {
                std::cout << "Failed to save file" << std::endl;
            }
            id |= static_cast<uint64_t>(1) << 51;

            if(encoder.AddFile(path + "Rm.wav", id, Encoding::PCM) != ErrorNum::NoErrors)
            {
                std::cout << "Failed to save file" << std::endl;
            }
        }
    }
    encoder.WritePackage(packOut);
    delete [] leftIR;
    delete [] rightIR;
    mysofa_close(hrtf);
    std::filesystem::remove_all("toDelete/");
    return 0;
}

int main(int argc, char** argv)
{
    if(argc < 4)
        std::cout << "not enough args" << std::endl;

    SOFAToPck(argv[1], argv[2], std::stoi(argv[3]));

    return 0;
}

