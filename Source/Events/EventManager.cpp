//
// Created by zack on 10/19/21.
//

#include "EventManager.h"
#include "cstring"

EventManager::EventManager(std::unordered_map<uint64_t, SoundData>& soundData, GameObjectManager& objectManager) :
                            eventID(100000), soundData(soundData), objectManager(objectManager)
{
    leftLocalBuffer = new float[buffSize];
    rightLocalBuffer = new float[buffSize];
}

EventManager::~EventManager()
{
    for(auto& event : events)
    {
        delete event.second;
    }
    delete [] leftLocalBuffer;
    delete [] rightLocalBuffer;
}

int EventManager::GetSamplesFromAllEvents(int numSamples, Frame<float> *buffer)
{
    // Clear entire  buffer, no need for any input data
    memset(buffer, 0, numSamples * sizeof(Frame<float>));

    int totalSamplesGenerated = 0;

    int generated = 0;
    while (numSamples - generated > 0)
    {
        int samplesToGet = numSamples;
        auto iter = events.begin();
        while(iter != events.end())
        {
            GameObject& eventObject = globalGameObject;
            objectManager.GetGameObject(iter->second->GetParent(), eventObject);

            // Clear local buffer for filters to use as needed
            memset(leftLocalBuffer, 0, samplesToGet * sizeof(float));
            memset(rightLocalBuffer, 0, samplesToGet * sizeof(float));

            int indexesFilled = iter->second->GetSamples(samplesToGet, leftLocalBuffer, rightLocalBuffer, eventObject);
            totalSamplesGenerated += indexesFilled;

            for (int i = 0; i < samplesToGet; ++i)
            {
                buffer[i + generated].leftChannel += leftLocalBuffer[i];
                buffer[i + generated].rightChannel += rightLocalBuffer[i];
            }

            if (indexesFilled == 0)
            {
                delete iter->second;
                iter = events.erase(iter);
                if (iter == events.end())
                    break;
                continue;
            }
            ++iter;
        }
        generated += samplesToGet;
    }
    if (events.size())
    {
        for (int i = 0; i < numSamples; ++i)
        {
            buffer[i].leftChannel = std::min(buffer[i].leftChannel, 1.0f);
            buffer[i].rightChannel = std::min(buffer[i].rightChannel, 1.0f);
        }
    }
    return totalSamplesGenerated;
}

void EventManager::ParseEvents(const std::string& path)
{
    eventParser.ParseEvents(path);
}

int EventManager::AddEvent(uint64_t id)
{
    Event* event;
    ErrorNum isValid = eventParser.GetEvent(id, &event, soundData);
    return AddEvent(event);
}

int EventManager::AddEvent(const std::string& name)
{
    Event* event;
    ErrorNum isValid = eventParser.GetEvent(name, &event, soundData);
    if(isValid != ErrorNum::NoErrors)
        return isValid;
    return AddEvent(event);
}

int EventManager::AddEvent(uint64_t id, uint64_t gameObjectId)
{
    Event* event;
    ErrorNum isValid = eventParser.GetEvent(id, &event, soundData);
    if(isValid != ErrorNum::NoErrors)
        return isValid;
    event->SetParent(gameObjectId);
    return AddEvent(event);
}

int EventManager::AddEvent(const std::string& name, uint64_t gameObjectId)
{
    Event* event;
    ErrorNum isValid = eventParser.GetEvent(name, &event, soundData);
    if(isValid != ErrorNum::NoErrors)
        return isValid;
    event->SetParent(gameObjectId);
    return AddEvent(event);
}