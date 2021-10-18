//
// Created by zack on 10/10/21.
//

#ifndef I_SOUND_ENGINE_OPUSCONTAINER_H
#define I_SOUND_ENGINE_OPUSCONTAINER_H

#include "SoundContainer.h"
#include "AudioFormats/OpusDecoderWrapper.h"
#include "AudioFormats/OpusHeader.h"
#include <cstring>
#include "AudioFormats/OpusFile.h"

constexpr int OpusFrameSize = 960;

template<typename sampleType>
class OpusContainer : public SoundContainer<sampleType>
{
public:

    OpusContainer(char* opusData, ChannelType channels) : type(channels),
                                          decoder(48000, channels == ChannelType::Mono ? 1 : 2),
                                          opusData(opusData),
                                          offsetIntoOpusFrame(std::numeric_limits<int>::max()),
                                          offsetIntoRawOpus(0)
    {

    }

    Frame<sampleType> GetNextSample() override
    {
        // Does a new frame need to be decoded
        if(offsetIntoOpusFrame >= OpusFrameSize * (type == ChannelType::Stereo ? 2 : 1))
        {
            // SanityCheck
            char OggSMagicNum[4];
            memcpy(OggSMagicNum, opusData + offsetIntoRawOpus, 4);

            assert(OggSMagicNum[0] == 'O'  && "OggS magic number not found in offset");

            int opusPacketSize;
            int tableIndex = OpusFile::GetSegementSize(opusData + offsetIntoRawOpus, opusPacketSize);

            offsetIntoRawOpus += tableIndex;
            int returnValue = decoder.Decode(opusData + offsetIntoRawOpus, opusPacketSize, decodedOpusFrame, OpusFrameSize);

            assert(returnValue >= 0 && "Opus decoder failed");

            offsetIntoRawOpus += opusPacketSize;
            offsetIntoOpusFrame = 0;
        }

        // Read samples into output
        Frame<sampleType> output;
        if(type == ChannelType::Mono)
        {
            output.leftChannel = decodedOpusFrame[offsetIntoOpusFrame];
            output.rightChannel = decodedOpusFrame[offsetIntoOpusFrame];
            ++offsetIntoOpusFrame;
        }
        else
        {
            output.leftChannel = decodedOpusFrame[offsetIntoOpusFrame];
            ++offsetIntoOpusFrame;
            output.rightChannel = decodedOpusFrame[offsetIntoOpusFrame];
            ++offsetIntoOpusFrame;
        }
        return output;
    }

private:
    ChannelType type;
    OpusDecoderWrapper decoder;

    char* opusData;
    uint64_t offsetIntoRawOpus;

    short decodedOpusFrame[OpusFrameSize * 2];
    int offsetIntoOpusFrame;

};

#endif //I_SOUND_ENGINE_OPUSCONTAINER_H
