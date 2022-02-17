//
// Created by zacke on 2/16/2022.
//

#ifndef I_SOUND_ENGINE_FIRSTORDERLOWPASS_H
#define I_SOUND_ENGINE_FIRSTORDERLOWPASS_H

#include "Biquad.h"

template<typename T>
class FirstOrderLowpass : public Biquad<T>
{
public:
    FirstOrderLowpass() : Biquad<T>()
    {
        SetCutoff(sampleRate/2.0);
    }

    void SetCutoff(T cutoff) override
    {
        if(cutoff <= 0)
            cutoff = 0;

        T theta = 2 * pi * (cutoff / sampleRate);
        T gama = std::cos(theta) / (1 + std::sin(theta));

        this->a0 = this->a1 = (1.0 - gama) / 2.0;
        this->b1 = -gama;
        this->c0 = 1;

        this->a2 = this->b2 = this->d0 = 0;
    }
};

#endif //I_SOUND_ENGINE_FIRSTORDERLOWPASS_H
