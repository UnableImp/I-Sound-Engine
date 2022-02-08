//
// Created by zack on 10/19/21.
//

#include "Event.h"

Event::Event(uint64_t gameObjectId) : gameObjectID(gameObjectId), eventID(0) {}

Event::Event(uint64_t gameObjectId, uint64_t eventID) : gameObjectID(gameObjectId), eventID(eventID) {}

Event::~Event()
{
    for(auto& filter : filters)
    {
        delete filter;
        filter = nullptr;
    }
}

void Event::AddFilter(Filter<float>* filter)
{
    filters.push_back(filter);
}

int Event::GetSamples(int numSamples, float* left, float* right, const GameObject& obj)
{
    int dataAdded = 0;
    for(const auto& filter: filters)
    {
        dataAdded += filter->GetNextSamples(numSamples, left, right, obj);
    }
    return dataAdded;
}

uint64_t Event::GetParent()
{
    return gameObjectID;
}

void Event::SetParent(uint64_t parent)
{
   gameObjectID = parent;
}

uint64_t Event::GetEventID()
{
    return eventID;
}