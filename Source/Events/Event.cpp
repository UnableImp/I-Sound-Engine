//
// Created by zack on 10/19/21.
//

#include "Event.h"
namespace ISoundEngine
{
Event::~Event()
{
    for (auto& filter : filters)
    {
        delete filter;
        filter = nullptr;
    }
}

void Event::AddFilter(Filter<float>* filter)
{
    filters.push_back(filter);
}

int Event::GetSamples(int numSamples, float* left, float* right)
{
    int dataAdded = 0;
    for (auto& filter : filters)
    {
        dataAdded += filter->GetNextSamples(numSamples, left, right);
    }
    return dataAdded;
}
}