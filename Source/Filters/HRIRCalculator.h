//
// Created by zack on 11/1/21.
//

#ifndef I_SOUND_ENGINE_HRIRCALCULATOR_H
#define I_SOUND_ENGINE_HRIRCALCULATOR_H

#include "Filter.h"
#include "SoundContainer.h"

template<typename sampleType>
class HRIRCalculator : public Filter<sampleType>
{
public:
    HRIRCalculator(/*listener ref, object ref,*/ SoundContainer<float>* left, SoundContainer<float>* right) : leftIR(left), rightIR(right) {}

    virtual ~HRIRCalculator() {}

    virtual int GetNextSamples(int numSamples, float* left, float* right) override
    {
        leftIR->GetNextSamples(numSamples, left, left);
        leftIR->Reset();
        rightIR->GetNextSamples(numSamples, right, right);
        rightIR->Reset();

        return 0;
    }

private:
    SoundContainer<float>* leftIR;
    SoundContainer<float>* rightIR;
};

#endif //I_SOUND_ENGINE_HRIRCALCULATOR_H
