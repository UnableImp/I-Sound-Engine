//
// Created by zacke on 2/17/2022.
//

#ifndef I_SOUND_ENGINE_SECONDORDERLOWPASS_H
#define I_SOUND_ENGINE_SECONDORDERLOWPASS_H

#include "Biquad.h"
#include "RealTimeParameters/GameObject.h"

template<typename T>
class SecondOrderLowpass : public Biquad<T>
{
public:
    SecondOrderLowpass() : Biquad<T>()
    {
        SetCutoff(sampleRate/2.0);
    }

    void SetCutoff(T cutoff) override
    {
        if(cutoff <= 0)
            cutoff = 1;

        T theta = 2 * pi * (cutoff / sampleRate);

        T d = 1 / GameObject::GetParam<float>("Q");

        T beta = 0.5 * ((1 - (d/2.0) * std::sin(theta)) /
                        (1 + (d/2.0) * std::sin(theta)));

        T gama = (0.5 + beta) * std::cos(theta);

        this->a0 = (0.5 + beta - gama) / 2.0;
        this->a1 = 0.5 + beta - gama;
        this->a2 = this->a0;

        this->b1 = -2.0 * gama;
        this->b2 = 2 * beta;
    }
};

#endif //I_SOUND_ENGINE_SECONDORDERLOWPASS_H
