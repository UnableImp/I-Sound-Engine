//
// Created by zack on 9/26/21.
//
#include "ContainerModule.h"
#include <string>
#include "AudioFormats/WavFile.h"

void readWaveFile(std::string name)
{
    //Opening test
    const char* path = name.c_str();
    WavFile wavFile(name);
    //if(!wavFile)
    ASSERT_FALSE(!wavFile) << "Failed to open file named ";


    // Conversion Test
    int samples = wavFile.GetDataSize() / (wavFile.GetFormat().bits_per_sample / 8);

    // Get Converted data
    float* floatSamples = new float[samples];
    uint32_t dataSize = samples * sizeof(float);

   char* nativeSamples = new char[wavFile.GetDataSize()];

    wavFile.GetDataAsFloat(floatSamples);

    wavFile.GetDataInNativeType(nativeSamples);

    switch (wavFile.GetFormat().bits_per_sample)
    {
        case 8:
            for(int i = 0; i < samples; ++i)
            {
                float f = (*((floatSamples) + i));
                short fValue = (f * (127)) ;
                fValue += (128);

                unsigned char sampleValue = *((nativeSamples) + i);
                ASSERT_TRUE((fValue >= sampleValue -1 && fValue <= sampleValue + 1)) << "Float conversion failed";
            }
            break;
        case 16:
            for(int i = 0; i < samples; ++i)
            {
                float f = (floatSamples)[i];
                short fValue = f * ((1<<15) - 1);

                short sampleValue = reinterpret_cast<short*>((nativeSamples))[i];
                ASSERT_TRUE((fValue >= sampleValue -1 && fValue <= sampleValue + 1)) << "Float conversion failed";
            }
            break;

        default:
            ASSERT_FALSE(true) << "Unsupported file format";
            return;
    }
    delete [] floatSamples;
    delete [] nativeSamples;
    return;
}

void encodeWaveToOggThenWav(const char* inFile, const char* outFileOgg, const char*  outFileWav)
{
    WavFile file(inFile);
    ASSERT_TRUE(file);

    char *output = new char[file.GetDataSize()]();
    // Write as ogg file to check compresstion ratio
    int fileSize = file.GetDataAsOpus(output);
    std::fstream oggFile(outFileOgg, std::ios_base::binary | std::ios_base::out);
    oggFile.write(output, fileSize);

    SoundData soundData;
    soundData.sampleCount = (file.GetDataSize() / 4 )  - 968;
    soundData.channels = file.GetFormat().channel_count;
    soundData.data =  output;

    OpusContainer<float> filter(soundData);

    //memset(data, 0, file.GetDataSize());
    char *data = new char[file.GetDataSize()]();

    for (int i = 0; i < file.GetDataSize(); i  += 4)
    {
        Frame<float> frame {0,0};
        filter.GetNextSamples(1, &frame.leftChannel, &frame.rightChannel);
        short  left = static_cast<short>(frame.leftChannel * (1 << 15));
        short  right = static_cast<short>(frame.rightChannel * (1 << 15));

        *reinterpret_cast<short*>(data + i) = left;
        *reinterpret_cast<short*>(data + i + 2) = right;
    }

    std::fstream tesConvert(outFileWav, std::ios_base::binary | std::ios_base::out);
    RiffHeader riffHeader{{'R','I','F','F'},
                          0,
                          {'W','A','V','E'}};
    tesConvert.write(reinterpret_cast<char*>(&riffHeader), sizeof(riffHeader));
    tesConvert.write(reinterpret_cast<const char*>(&file.GetFormat()), sizeof(FormatHeader));
    GenericHeaderChunk dataChunk{{'d', 'a','t','a'}, file.GetDataSize()};
    dataChunk.chunkSize = file.GetDataSize();
    tesConvert.write(reinterpret_cast<char*>(&dataChunk), sizeof(dataChunk));
    tesConvert.write(data, file.GetDataSize());

    delete [] output;
    delete [] data;
}