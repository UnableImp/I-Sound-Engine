//
// Created by zack on 11/25/21.
//

#ifndef I_SOUND_ENGINE_POSTEVENT_H
#define I_SOUND_ENGINE_POSTEVENT_H

#include "Action.h"
#include "Events/Event.h"

class PostEventAction : public Action
{
public:

    PostEventAction(uint64_t actionId, Event* event, uint64_t eventID) : Action(actionId), event(event), eventID(eventID) {}

    ActionType GetActionType() override
    {
        return PostEvent;
    }

    Event* GetEvent()
    {
        return event;
    }

    uint64_t GetEventId()
    {
        return eventID;
    }

private:
    Event* event;
    uint64_t eventID;
};

#endif //I_SOUND_ENGINE_POSTEVENT_H
