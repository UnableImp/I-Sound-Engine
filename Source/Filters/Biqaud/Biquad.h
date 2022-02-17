//
// Created by zacke on 2/16/2022.
//

#ifndef I_SOUND_ENGINE_BIQUAD_H
#define I_SOUND_ENGINE_BIQUAD_H

#include "../Filter.h"
#include "../../Constants.h"
#include <cmath>

template<typename T>
class Biquad : public Filter<T>
{
public:
    virtual ~Biquad() {}
    /*!
     * Fills a buffer with audio samples, if no audio data is available zeros are filled
     * @param numSamples Number of samples to fill buffer with
     * @param buffer Buffer to fill
     * @return Number of samples filled
     */
    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        for(int i = 0; i < numSamples; ++i)
        {
            float x = left[i];

            // Biqaud equation
             float y = (a0 * x)  +
                       (a1 * x1) +
                       (a2 * x2) -
                       (b1 * y1) -
                       (b2 * y2);

            left[i] = y;

            // Move forward
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
        }
        return 0;
    };

    virtual void SetCutoff(T cutoff) = 0;

protected:
    T a0;
    T a1;
    T a2;
    T b1;
    T b2;
    T c0;
    T d0;

    T x1;
    T x2;
    T y1;
    T y2;
};

#endif //I_SOUND_ENGINE_BIQUAD_H
