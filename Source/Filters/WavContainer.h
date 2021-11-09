/******************************************************************************/
/*!
\file   WavContainer.h
\author Zack Krolikowksi
\date   8/22/2021

    Container for wav files
*/
/******************************************************************************/

#ifndef I_SOUND_ENGINE_WAVCONTAINER_H
#define I_SOUND_ENGINE_WAVCONTAINER_H

#include "SoundData.h"
#include "SoundContainer.h"

#include <iostream>

template<typename sampleType>
class WavContainer : public SoundContainer<sampleType>
{
public:
    WavContainer(SoundData& data) : SoundContainer<sampleType>(), data(data)
    {
        totalOffset = 0;
    }

    virtual void Reset() override
    {
        totalOffset = 0;
        this->RandomPitch();
    }

    // Fully contaioned sound object
    //WavContainer()


    virtual int GetNextSamples(int numSamples, float* left, float* right) override
    {
        int frames = 0;
        for(int i = 0; i < numSamples; ++i)
        {
            if(totalOffset >= data.sampleCount - 2)
            {
                if(this->totalLoops == -1)
                {
                    Reset();
                }
                else if(this->currentLoopCount < this->totalLoops)
                {
                    Reset();
                    ++this->currentLoopCount;
                }
                else
                {
                    this->FillZeros(numSamples - i, left + i, right + i);
                    return frames;
                }
            }

            if (data.channels == ChannelType::Mono)
            {
                float* sampleArray = reinterpret_cast<float*>(data.data);

                sampleType value = this->lerp(sampleArray[static_cast<int>(totalOffset)],
                                              sampleArray[static_cast<int>(totalOffset) + 2],
                                              totalOffset - static_cast<int>(totalOffset));

                left[i] += value;
                right[i] += value;
                totalOffset += this->playbackModifier;
            }
            else
            {
                float* sampleArray = reinterpret_cast<float*>(data.data);

                sampleType value = this->lerp(sampleArray[static_cast<int>(totalOffset)],
                                              sampleArray[static_cast<int>(totalOffset) + 2],
                                              totalOffset - static_cast<int>(totalOffset));

                left[i] += value;
                totalOffset += 1;

                value = this->lerp(sampleArray[static_cast<int>(totalOffset)],
                                   sampleArray[static_cast<int>(totalOffset) + 2],
                                   totalOffset - static_cast<int>(totalOffset));

                right[i] += value;
                totalOffset += this->playbackModifier;
            }
            ++frames;
        }
        return frames;
    }

    void Seek(int position)
    {

    }

private:
    double totalOffset = 0;
    SoundData& data;
};

#endif //I_SOUND_ENGINE_WAVCONTAINER_H
