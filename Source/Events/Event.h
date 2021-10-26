//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_EVENT_H
#define I_SOUND_ENGINE_EVENT_H

#include "Filters/Filter.h"
#include "AudioFrame.h"
#include <vector>
namespace ISoundEngine
{
class Event
{
public:
    ~Event();

    void AddFilter(Filter<float>* filter);

    int GetSamples(int numSamples, float* left, float* right);

private:
    std::vector<Filter<float>*> filters;
};
}
#endif //I_SOUND_ENGINE_EVENT_H
