//
// Created by zack on 11/25/21.
//

#ifndef I_SOUND_ENGINE_ACTION_H
#define I_SOUND_ENGINE_ACTION_H
#include <inttypes.h>

enum ActionType
{
    PostEvent,
    StopEvent
};

class Action
{
public:
    Action(uint64_t actionIndex) : actionIndex(actionIndex) {}

    virtual ActionType GetActionType() = 0;

    uint64_t GetActionIndex()
    {
        return actionIndex;
    }

private:
    uint64_t actionIndex;
};

#endif //I_SOUND_ENGINE_ACTION_H
