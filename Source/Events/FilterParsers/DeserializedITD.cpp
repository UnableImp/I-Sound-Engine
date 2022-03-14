//
// Created by zacke on 2/5/2022.
//

#include "DeserializedITD.h"
#include "Filters/ITD.h"
#include "Filters/Filter.h"
#include "RealTimeParameters/GameObject.h"

DeserializedITD::DeserializedITD(rapidjson::Value &object)
{}

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
    return BuildFilter(filter, manager);
}