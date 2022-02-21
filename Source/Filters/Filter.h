//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_FILTER_H
#define I_SOUND_ENGINE_FILTER_H

#include "AudioFrame.h"
#include "RealTimeParameters/GameObject.h"

#include <complex>

template<typename sampleType>
class Filter
{
public:
    virtual ~Filter() {}
    /*!
     * Fills a buffer with audio samples, if no audio data is available zeros are filled
     * @param numSamples Number of samples to fill buffer with
     * @param buffer Buffer to fill
     * @return Number of samples filled
     */
    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj) {return 0;};

protected:
    inline static sampleType lerp(sampleType a, sampleType b, float t)
    {
        return a+(t*(b-a));
    }

    inline static std::complex<float> lerp(std::complex<float>& a, std::complex<float>& b, float t)
    {
        return std::polar(lerp(std::abs(a), std::abs(b), t), lerp(std::arg(a), std::arg(b), t));
    }

    inline static sampleType EaseInQuad(sampleType t)
    {
        return t*t;
    }
    inline static sampleType EaseInCubic(sampleType t)
    {
        return t*t*t;
    }
    inline static sampleType EaseInQuart(sampleType t)
    {
        return t*t*t*t;
    }

    inline sampleType EaseOutQuad(sampleType t)
    {
        return 1 - ((1 - t) * (1 - t));
    }
    inline sampleType EaseOutCubic(sampleType t)
    {
        return 1 - ((1-t)*(1-t)*(1-t)*(1-t));
    }
    inline sampleType EaseOutQuart(sampleType t)
    {
        return 1 - std::pow(2, -10*t);
    }

};

#endif //I_SOUND_ENGINE_FILTER_H
