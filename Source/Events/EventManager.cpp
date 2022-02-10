//
// Created by zack on 10/19/21.
//

#include "EventManager.h"
#include "cstring"
#include "Actions/Action.h"
#include "Actions/PostEvent.h"
#include "Actions/StopEvent.h"
#include <chrono>
#include <iostream>

EventManager::EventManager(PackageManager& soundData, GameObjectManager& objectManager) :
                            eventID(100000), soundData(soundData), objectManager(objectManager),
                            nextActionID(1), lastActedID(0)
{
    leftLocalBuffer = new float[buffSize]();
    rightLocalBuffer = new float[buffSize]();
}

EventManager::~EventManager()
{
    for (auto &event: events)
    {
        delete event.second;
    }
    for (auto &action: actionList)
    {
        delete action;
    }

    delete [] leftLocalBuffer;
    delete [] rightLocalBuffer;
}

void EventManager::Update()
{
    auto itemToRemove = actionList.begin();
    while (itemToRemove != actionList.end() && (*itemToRemove)->GetActionIndex() < lastActedID)
    {
        delete *itemToRemove;
        itemToRemove = actionList.erase(itemToRemove);
    }
}

int EventManager::GetSamplesFromAllEvents(int numSamples, Frame<float> *buffer)
{
    auto start = std::chrono::high_resolution_clock::now();
    GameObject::SetParam("HRTFLoadTemp", 0.0f);
    GameObject::SetParam("ITDLoadTemp", 0.0f);
    // Clear entire  buffer, no need for any input data
    memset(buffer, 0, numSamples * sizeof(Frame<float>));

    int totalSamplesGenerated = 0;

    for(auto iter = actionList.begin(); iter != actionList.end(); ++iter)
    {
        if((*iter)->GetActionIndex() > lastActedID)
        {
            lastActedID = (*iter)->GetActionIndex();
            switch((*iter)->GetActionType())
            {
                case ActionType::PostEvent:
                {
                    PostEventAction* action = dynamic_cast<PostEventAction*>(*iter);
                    events[action->GetEventId()] = action->GetEvent();
                    break;
                }
                case ActionType::StopEvent:
                {
                    StopEventAction* action = dynamic_cast<StopEventAction*>(*iter);
                    for (auto event = events.begin(); event != events.end(); ++event)
                    {
                        if(event->second->GetEventID() == action->GetEventId())
                        {
                            events.erase(event);
                            break;
                        }
                    }
                }
                default:
                    assert("Unknown action type in buffer");
            }
        }
    }

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
                continue;
            }
            ++iter;
        }
        generated += samplesToGet;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> seconds = end-start;
    GameObject::SetParam("DSPLoad", seconds.count() / (512.0f/44100.0f));
    GameObject::SetParam("HRTFLoad", GameObject::GetParam<float>("HRTFLoadTemp") / (512.0f / 44100.0f));
    GameObject::SetParam("ITDLoad", GameObject::GetParam<float>("ITDLoadTemp") / (512.0f / 44100.0f));
    GameObject::SetParam("RunningObjs", static_cast<float>(events.size()));
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
    if(isValid == ErrorNum::NoErrors)
        return AddEvent(event);
    return isValid;
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

void EventManager::StopEvent(uint64_t eventID)
{
    Action* action = new StopEventAction(nextActionID, eventID);
    actionList.push_back(action);

    ++nextActionID;
}