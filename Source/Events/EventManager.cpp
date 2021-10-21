//
// Created by zack on 10/19/21.
//

#include "EventManager.h"
#include "cstring"

EventManager::EventManager() : eventID(100000)
{}

int EventManager::GetSamplesFromAllEvents(int numSamples, Frame<float> *buffer)
{
    // Clear entire  buffer, no need for any input data
    memset(buffer, 0, numSamples * sizeof(Frame<float>));

    int totalSamplesGenerated = 0;

    int generated = 0;
    while(numSamples - generated > 0)
    {
        int samplesToGet = numSamples;
        for(auto iter = events.begin(); iter != events.end(); ++iter)
        {
            // Clear local buffer for filters to use as needed
            memset(localBuffer, 0, samplesToGet * sizeof(Frame<float>));

            int indexesFilled = iter->second->GetSamples(samplesToGet, localBuffer);
            totalSamplesGenerated += indexesFilled;

            for(int i = 0; i < samplesToGet; ++i)
            {
                buffer[i + generated] += (localBuffer[i]);
            }

            if(indexesFilled == 0)
            {
                delete iter->second;
                iter = events.erase(iter);
                if (iter == events.end())
                    break;
            }
        }
        generated += samplesToGet;
    }
    for(int i = 0; i < numSamples; ++i)
    {
        buffer[i] /= static_cast<int>(events.size());
    }
    return totalSamplesGenerated;
}