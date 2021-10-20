//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_FILTER_H
#define I_SOUND_ENGINE_FILTER_H

#include "AudioFrame.h"

template<typename sampleType>
class Filter
{
public:
    /*!
     * Fills a buffer with audio samples, if no audio data is available zeros are filled
     * @param numSamples Number of samples to fill buffer with
     * @param buffer Buffer to fill
     * @return Number of samples filled
     */
    virtual int GetNextSamples(int numSamples, Frame<sampleType>* buffer) = 0;
};

#endif //I_SOUND_ENGINE_FILTER_H
