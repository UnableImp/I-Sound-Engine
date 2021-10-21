//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_EVENTMANAGER_H
#define I_SOUND_ENGINE_EVENTMANAGER_H

#include "Event.h"
#include "Filters/Filter.h"
#include <unordered_map>

constexpr int buffSize =  1024  ;

class EventManager
{
public:
    EventManager();

    /*!
     * TEMPERARY untill a json or such system is set up to read events from
     * @tparam T all filters to add types
     * @param filters Filters to add
     * @return event ID
     */
    template<typename... T>
    int AddEvent(Filter<float>* filter, T... filters)
    {
        Event* newEvent = new Event();
        return AddEvent(newEvent, filter, filters...);
    }

    /*!
     * Fills buffer with data from  all data
     * @param numSamples
     * @param buffer
     * @return
     */
    int GetSamplesFromAllEvents(int numSamples, Frame<float>* buffer);

private:

    template<typename... T>
    int AddEvent(Event* event, Filter<float>* filter, T... filters)
    {
        event->AddFilter(filter);
        return AddEvent(event, filters...);
    }

    int AddEvent(Event* event)
    {
        ++eventID;
        events[eventID] = event;
        return eventID;
    }

    int eventID;
    std::unordered_map<int, Event*> events; //!< TODO  MAKE THREAD SAFE
    Frame<float> localBuffer[buffSize];
};


#endif //I_SOUND_ENGINE_EVENTMANAGER_H
