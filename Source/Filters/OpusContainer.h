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

    OpusContainer(SoundData& data) : SoundContainer<sampleType>(),
                                    data(data),
                                    decoder(48000, data.channels == ChannelType::Mono ? 1 : 2),
                                    offsetIntoOpusFrame(std::numeric_limits<int>::max()),
                                    offsetIntoRawOpus(0),
                                    totalOffset(0)
    {

    }

    virtual void Reset() override
    {
        offsetIntoOpusFrame = std::numeric_limits<int>::max();
        offsetIntoRawOpus = 0;
        totalOffset = 0;
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right) override
    {
        int frames = 0;
        for(int i = 0; i < numSamples; ++i)
        {
            if(totalOffset >= data.sampleCount)
            {
                if(this->totalLoops == -1)
                {
                    Reset();
                }
                else if(this->currentLoopCount < this->totalLoops)
                {
                    ++this->currentLoopCount;
                    Reset();
                }
                else
                {
                    this->FillZeros(numSamples - i, left + i, right + i);
                    return frames;
                }
            }
            // Does a new frame need to be decoded
            if (offsetIntoOpusFrame >= (OpusFrameSize * (data.channels == ChannelType::Stereo ? 2 : 1)))
            {
                decodeFrame();
            }

            // Read samples into output
            if (data.channels == ChannelType::Mono)
            {
                sampleType value = this->lerp(decodedOpusFrame[static_cast<int>(offsetIntoOpusFrame)],
                                              decodedOpusFrame[static_cast<int>(offsetIntoOpusFrame) + 2],
                                              totalOffset - static_cast<int>(totalOffset));

                left[i] += value;
                right[i] += value;
                ++offsetIntoOpusFrame;
            } else
            {
                sampleType value = this->lerp(decodedOpusFrame[static_cast<int>(offsetIntoOpusFrame)],
                                              decodedOpusFrame[static_cast<int>(offsetIntoOpusFrame) + 2],
                                              totalOffset - static_cast<int>(totalOffset));
                left[i] += value;
                ++offsetIntoOpusFrame;

                value = this->lerp(decodedOpusFrame[static_cast<int>(offsetIntoOpusFrame)],
                                   decodedOpusFrame[static_cast<int>(offsetIntoOpusFrame) + 2],
                                   totalOffset - static_cast<int>(totalOffset));
                right[i] += value;
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
        int tableIndex = OpusFile::GetSegementSize(data.data + static_cast<int>(offsetIntoRawOpus), opusPacketSize);

        // Read packet
        offsetIntoRawOpus += tableIndex;
        int returnValue = decoder.DecodeFloat(data.data + static_cast<int>(offsetIntoRawOpus), opusPacketSize, decodedOpusFrame, OpusFrameSize);

        assert(returnValue >= 0 && "Opus decoder failed");

        // Setup return values
        offsetIntoRawOpus += opusPacketSize;
        offsetIntoOpusFrame = totalOffset - static_cast<int>(totalOffset);
    }

    SoundData data;
    OpusDecoderWrapper decoder;

    int offsetIntoRawOpus;

    float decodedOpusFrame[OpusFrameSize * 2 + 1];
    double offsetIntoOpusFrame;

    double totalOffset;
};

#endif //I_SOUND_ENGINE_OPUSCONTAINER_H
