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

template<typename sampleType>
class WavContainer : public SoundContainer<sampleType>
{
public:
    WavContainer(SoundData& data) : data(data)
    {
        totalOffset = 0;
    }

    // Fully contaioned sound object
    //WavContainer()


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

            if (data.channels == ChannelType::Mono)
            {
                buffer[i].leftChannel += reinterpret_cast<float *>(data.data)[totalOffset];
                buffer[i].rightChannel += reinterpret_cast<float *>(data.data)[totalOffset];
                ++totalOffset;
            } else
            {
                buffer[i].leftChannel += reinterpret_cast<float *>(data.data)[totalOffset];
                ++totalOffset;
                buffer[i].rightChannel += reinterpret_cast<float *>(data.data)[totalOffset];
                ++totalOffset;
            }
            ++frames;
        }
        return frames;
    }

    void Seek(int position)
    {

    }

private:
    int totalOffset = 0;
    SoundData& data;
};

#endif //I_SOUND_ENGINE_WAVCONTAINER_H
