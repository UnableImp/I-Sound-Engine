//
// Created by zack on 10/21/21.
//

#ifndef I_SOUND_ENGINE_EVENTPARSER_H
#define I_SOUND_ENGINE_EVENTPARSER_H

#include <string>
#include "Event.h"
#include "ErrorList.h"
#include <unordered_map>
#include <vector>
#include "FilterParsers/DeserializeFilter.h"
#include "AudioPackage/PackageManager.h"

class EventParser
{
public:
    void ParseEvents(const std::string& path);

    ErrorNum GetEvent(const std::string& name, Event** event,  PackageManager& manager);
    ErrorNum GetEvent(uint64_t id, Event** Event, PackageManager& manager);

    ErrorNum GetEvent(const std::string& name, Event** event,  PackageManager& manager, GameObject& obj);
    ErrorNum GetEvent(uint64_t id, Event** Event, PackageManager& manager, GameObject& obj);

    ~EventParser();

private:
    std::unordered_map<std::string, uint64_t> stringToId;
    std::unordered_map<uint64_t, std::vector<DeserializedFilter*>> IdToEvent;
};


#endif //I_SOUND_ENGINE_EVENTPARSER_H
