//
// Created by zacke on 2/5/2022.
//

#include "DeserializedITD.h"
#include "Filters/ITD.h"
#include "Filters/Filter.h"
#include "RealTimeParameters/GameObject.h"

DeserializedITD::DeserializedITD(rapidjson::Value &object) : dopplerStength(-1)
{
    if(object.HasMember("Doppler"))
        dopplerStength = object["Doppler"].GetFloat();
}

ErrorNum DeserializedITD::BuildFilter(Filter<float> **filter,  PackageManager& manager)
{
    if(GameObject::GetParamStatic<float>("UseITD"))
    {
        *filter = new ITD();
    }
    else
    {
        *filter = new Filter<float>();
    }
    return NoErrors;
}

ErrorNum DeserializedITD::BuildFilter(Filter<float>** filter,  PackageManager& manager, GameObject& obj)
{
    if(dopplerStength > -1)
        obj.SetParamLocal("DistanceScaler", dopplerStength);
    return BuildFilter(filter, manager);
}