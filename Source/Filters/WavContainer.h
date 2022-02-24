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
    }

    // Fully contaioned sound object
    //WavContainer()


    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj) override
    {
      if(this->playbackModifier == 1.0f)
      {
          if (data.channels == ChannelType::Mono)
          {
              return GetNextSamplesStatic(numSamples, left, right, obj);
          }
          else
          {
              return GetNextSamplesStaticStereo(numSamples, left, right, obj);
          }
      }
      return GetNextSamplesNonStatic(numSamples, left, right, obj);
    }

    void Seek(int position)
    {

    }

private:

    int GetNextSamplesStaticStereo(int numSamples, float* left, float* right, const GameObject& obj)
    {
        int toRead = numSamples * 2;
        if(toRead + totalOffset >= data.sampleCount - 4)
        {
            toRead = ((data.sampleCount - 4) - totalOffset) / 2.0f;
        }

        for(int i = 0; i < toRead/2; ++i)
        {
            float* sampleArray = reinterpret_cast<float*>(data.data);

            left[i] += sampleArray[static_cast<int>(totalOffset)];;
            right[i] += sampleArray[static_cast<int>(totalOffset + 1)];;

            totalOffset += 2;
        }

        if(toRead != numSamples*2)
        {
            if(this->totalLoops == -1)
            {
                Reset();
                toRead += GetNextSamplesStaticStereo(numSamples - toRead, left+toRead, right+toRead, obj);
            }
            else if(this->currentLoopCount < this->totalLoops)
            {
                Reset();
                ++this->currentLoopCount;
                toRead += GetNextSamplesStaticStereo(numSamples - toRead, left+toRead, right+toRead, obj);
            }
            else
            {
                return toRead;
            }
        }

        return toRead;
    }

    int GetNextSamplesNonStatic(int numSamples, float* left, float* right, const GameObject& obj)
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

    int GetNextSamplesStatic(int numSamples, float* left, float* right, const GameObject& obj)
    {
        int toRead = numSamples;
        if(toRead + totalOffset >= data.sampleCount - 2)
        {
            toRead = (data.sampleCount - 2) - totalOffset;
        }

        for(int i = 0; i < toRead; ++i)
        {
            float* sampleArray = reinterpret_cast<float*>(data.data);

            left[i] += sampleArray[static_cast<int>(totalOffset)];;
            right[i] += sampleArray[static_cast<int>(totalOffset)];;

            totalOffset += 1;
        }

        if(toRead != numSamples)
        {
            if(this->totalLoops == -1)
            {
                Reset();
                toRead += GetNextSamplesStaticStereo(numSamples - toRead, left+toRead, right+toRead, obj);
            }
            else if(this->currentLoopCount < this->totalLoops)
            {
                Reset();
                ++this->currentLoopCount;
                toRead += GetNextSamplesStaticStereo(numSamples - toRead, left+toRead, right+toRead, obj);
            }
            else
            {
                return 0;
            }
        }

        return toRead;
    }


    double totalOffset = 0;
    SoundData& data;
};

#endif //I_SOUND_ENGINE_WAVCONTAINER_H
