//
// Created by zacke on 2/17/2022.
//

#ifndef I_SOUND_ENGINE_LINKWITZRILEYLOWPASS_H
#define I_SOUND_ENGINE_LINKWITZRILEYLOWPASS_H


#include "Biquad.h"
#include "RealTimeParameters/GameObject.h"

template<typename T>
class LinkwitzRileyLowpass : public Biquad<T>
{
public:
    LinkwitzRileyLowpass() : Biquad<T>()
    {
        SetCutoff(sampleRate/2.0);
    }

    void SetCutoff(T cutoff) override
    {
        if(cutoff <= 0)
            cutoff = 0;

        T theta = pi * (cutoff / sampleRate);
        T omega = pi * cutoff;

        T k = omega / std::tan(theta);

        T delta = (k * k) + (omega * omega) + (2 * k * omega);

        this->a0 = (omega * omega) / delta;
        this->a1 = 2 * this->a0;
        this->a2 = this->a0;

        this->b1 = ((-2 * k * k) + (2 * omega * omega)) / delta;
        this->b2 =((-2 * k * omega) + (k * k) + (omega * omega)) / delta;
    }
};

#endif //I_SOUND_ENGINE_LINKWITZRILEYLOWPASS_H
