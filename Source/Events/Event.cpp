//
// Created by zack on 10/19/21.
//

#include "Event.h"

void Event::AddFilter(Filter<float>* filter)
{
    filters.push_back(filter);
}

int Event::GetSamples(int numSamples, Frame<float> *frame)
{
    int dataAdded = 0;
    for(auto& filter: filters)
    {
        dataAdded += filter->GetNextSamples(numSamples, frame);
    }
    return dataAdded;
}