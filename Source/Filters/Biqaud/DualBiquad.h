//
// Created by zacke on 2/16/2022.
//

#ifndef I_SOUND_ENGINE_DUALBIQUAD_H
#define I_SOUND_ENGINE_DUALBIQUAD_H

#include "Biquad.h"

template<typename T>
class DualBiquad : public Biquad<T>
{
public:
    DualBiquad(Biquad<T>* leftQaud, Biquad<T>* rightQaud) : leftQaud(leftQaud), rightQaud(rightQaud)
    {

    }

    ~DualBiquad()
    {
        delete leftQaud;
        delete rightQaud;
    }

    int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj) override
    {
        leftQaud->GetNextSamples(numSamples, left, right, obj);
        rightQaud->GetNextSamples(numSamples, right, left, obj);

        return 0;
    }

    void SetCutoff(T cutoff) override
    {
        leftQaud->SetCutoff(cutoff);
        rightQaud->SetCutoff(cutoff);
    }

private:
    Biquad<T>* leftQaud;
    Biquad<T>* rightQaud;
};


#endif //I_SOUND_ENGINE_DUALBIQUAD_H
