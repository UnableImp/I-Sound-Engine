//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_EVENT_H
#define I_SOUND_ENGINE_EVENT_H

#include "Filters/Filter.h"
#include "AudioFrame.h"
#include <vector>
#include <cstdint>

class Event
{
public:

    Event(uint64_t gameObjectId);

    Event(uint64_t gameObjectId, uint64_t eventID);

    ~Event();

    void AddFilter(Filter<float>* filter);

    int GetSamples(int numSamples, float* left, float* right, const GameObject& obj);

    uint64_t GetParent();
    void SetParent(uint64_t parent);

    uint64_t GetEventID();

private:
    std::vector<Filter<float> *> filters;
    uint64_t gameObjectID;
    uint64_t eventID;
};

#endif //I_SOUND_ENGINE_EVENT_H
