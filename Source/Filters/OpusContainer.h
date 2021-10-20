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
#include "SoundData.h"

constexpr int OpusFrameSize = 960;

template<typename sampleType>
class OpusContainer : public SoundContainer<sampleType>
{
public:

    OpusContainer(SoundData& data) : data(data),
                                    decoder(48000, data.channels == ChannelType::Mono ? 1 : 2),
                                    offsetIntoOpusFrame(std::numeric_limits<int>::max()),
                                    offsetIntoRawOpus(0),
                                    totalOffset(0)
    {

    }

    virtual int GetNextSamples(int numSamples, Frame<sampleType>* buffer) override
    {
        int frames = 0;
        for(int i = 0; i < numSamples; ++i)
        {
            if(totalOffset >= data.sampleCount)
            {
                this->FillZeros(numSamples - i, buffer + i);
                return frames;
            }
            // Does a new frame need to be decoded
            if (offsetIntoOpusFrame >= OpusFrameSize * (data.channels == ChannelType::Stereo ? 2 : 1))
            {
                decodeFrame();
            }

            // Read samples into output
            if (data.channels == ChannelType::Mono)
            {
                buffer[i].leftChannel += decodedOpusFrame[offsetIntoOpusFrame];
                buffer[i].rightChannel += decodedOpusFrame[offsetIntoOpusFrame];
                ++offsetIntoOpusFrame;
            } else
            {
                buffer[i].leftChannel += decodedOpusFrame[offsetIntoOpusFrame];
                ++offsetIntoOpusFrame;
                buffer[i].rightChannel += decodedOpusFrame[offsetIntoOpusFrame];
                ++offsetIntoOpusFrame;
            }
            ++totalOffset;
            ++frames;
        }
        return frames;
    }

private:

    void decodeFrame()
    {
        // SanityCheck
        assert(*(data.data + offsetIntoRawOpus) == 'O'  && "OggS magic number not found in offset");

        // find size of next opus packet
        int opusPacketSize;
        int tableIndex = OpusFile::GetSegementSize(data.data + offsetIntoRawOpus, opusPacketSize);

        // Read packet
        offsetIntoRawOpus += tableIndex;
        int returnValue = decoder.DecodeFloat(data.data + offsetIntoRawOpus, opusPacketSize, decodedOpusFrame, OpusFrameSize);

        assert(returnValue >= 0 && "Opus decoder failed");

        offsetIntoRawOpus += opusPacketSize;
        offsetIntoOpusFrame = 0;
    }

    SoundData data;
    OpusDecoderWrapper decoder;

    uint64_t offsetIntoRawOpus;

    float decodedOpusFrame[OpusFrameSize * 2];
    int offsetIntoOpusFrame;

    uint64_t totalOffset;
};

#endif //I_SOUND_ENGINE_OPUSCONTAINER_H
