//
// Created by zack on 11/25/21.
//

#ifndef I_SOUND_ENGINE_STOPEVENT_H
#define I_SOUND_ENGINE_STOPEVENT_H

#include "Action.h"
#include "Events/Event.h"

class StopEventAction : public Action
{
public:

    StopEventAction(uint64_t actionId, uint64_t eventID) : Action(actionId), eventID(eventID) {}

    ActionType GetActionType() override
    {
        return StopEvent;
    }

    uint64_t GetEventId()
    {
        return eventID;
    }

private:
    uint64_t eventID;
};

#endif //I_SOUND_ENGINE_STOPEVENT_H
