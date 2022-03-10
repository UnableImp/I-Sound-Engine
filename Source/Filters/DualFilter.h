//
// Created by zacke on 2/21/2022.
//

#ifndef I_SOUND_ENGINE_DUALFILTER_H
#define I_SOUND_ENGINE_DUALFILTER_H

#include "Filter.h"

class DualFilter : public Filter<float>
{
public:
    DualFilter(Filter<float> *left, Filter<float> *right) : filterLeft(left), filterRight(right)
    {}

    ~DualFilter()
    {
        delete filterRight;
        delete filterLeft;
    }

    int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        return std::max( filterRight->GetNextSamples(numSamples,right,right, obj),
                         filterLeft ->GetNextSamples(numSamples,left, left,  obj));
    }
private:
    Filter<float>* filterLeft;
    Filter<float>* filterRight;
};

#endif //I_SOUND_ENGINE_DUALFILTER_H
