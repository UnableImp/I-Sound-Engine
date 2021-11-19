//
// Created by zack on 10/21/21.
//

#include "EventParser.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <cstdio>
#include "FilterParsers/DeserializedSound.h"
#include "Event.h"
#include "FilterParsers/Deserialized3DSound.h"

constexpr const char* id = "id";

void EventParser::ParseEvents(const std::string& path)
{
    FILE* fp = fopen(path.c_str(), "rb");

    assert(fp && (std::string("JSON File not found:") + path).c_str());

    char readBuffer[4096];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    rapidjson::Document document;
    document.SetObject();
    document.ParseStream(is);

    fclose(fp);

    for(auto eventSerlized = document.MemberBegin(); eventSerlized != document.MemberEnd(); ++eventSerlized)
    {
        std::string eventName(eventSerlized->name.GetString());
        uint64_t eventID = eventSerlized->value.GetObject()["ID"].GetUint64();

        stringToId[eventName] = eventID;
        IdToEvent[eventID] = {};

        for(auto filter = eventSerlized->value.GetObject().MemberBegin(); filter != eventSerlized->value.GetObject().MemberEnd(); ++filter)
        {
            if(std::string("Sound") == filter->name.GetString())
            {
                IdToEvent[eventID].push_back(new DeserializedSound(filter->value));
            }
            if(std::string("3D") == filter->name.GetString())
            {
                IdToEvent[eventID].push_back(new Deserialize3DSound(filter->value));
            }
        }
    }
}

ErrorNum EventParser::GetEvent(const std::string& name, Event **event,  PackageManager& manager)
{
    return GetEvent(stringToId[name], event, manager);
}

ErrorNum EventParser::GetEvent(uint64_t id, Event **event,  PackageManager& manager)
{
    if(IdToEvent.find(id) == IdToEvent.end())
        return ErrorNum::EventNotFound;

    *event = new Event(0);
    auto& filtersInEvent = IdToEvent[id];

    for(auto sFilter : filtersInEvent)
    {
        Filter<float>* filter;
        sFilter->BuildFilter(&filter, manager);

        (*event)->AddFilter(filter);
    }
    return ErrorNum::NoErrors;
}

EventParser::~EventParser()
{
    for(auto& event : IdToEvent)
    {
        for(auto& filter : event.second)
        {
            delete filter;
        }
    }
}