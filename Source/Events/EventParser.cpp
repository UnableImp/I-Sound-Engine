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

constexpr const char* id = "id";

void EventParser::ParseEvents(const std::string& path)
{
    FILE* fp = fopen(path.c_str(), "rb");

    assert(fp && "JSON File not found");

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
        }
    }
}

ErrorNum EventParser::GetEvent(const std::string& name, Event **event, std::unordered_map<uint64_t, SoundData>& soundData)
{
    return GetEvent(stringToId[name], event, soundData);
}

ErrorNum EventParser::GetEvent(uint64_t id, Event **event, std::unordered_map<uint64_t, SoundData>& soundData)
{
    if(IdToEvent.find(id) == IdToEvent.end())
        return ErrorNum::EventNotFound;

    *event = new Event;
    auto& filtersInEvent = IdToEvent[id];

    for(auto sFilter : filtersInEvent)
    {
        Filter<float>* filter;
        sFilter->BuildFilter(&filter, soundData);

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