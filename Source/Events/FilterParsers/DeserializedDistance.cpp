//
// Created by zacke on 3/2/2022.
//

#include "DeserializedDistance.h"
#include "Filters/DistanceAttenuation.h"
#include "Filters/Biqaud/FirstOrderLowpass.h"
#include "Filters/Biqaud/SecondOrderLowpass.h"
#include "Filters/Biqaud/LinkwitzRileyLowpass.h"

DeserializedDistance::DeserializedDistance(rapidjson::Value& object)
{

}

ErrorNum DeserializedDistance::BuildFilter(Filter<float>** filter,  PackageManager& manager)
{
    if(GameObject::GetParamStatic<float>("UseDistanceAtten"))
    {
        Biquad<float>* lowpass;
        switch (static_cast<int>(GameObject::GetParamStatic<float>("LowpassType")))
        {
            case 0:
                lowpass = new FirstOrderLowpass<float>;
                break;
            case 1:
                lowpass = new SecondOrderLowpass<float>;
                break;
            case 2:
                lowpass = new LinkwitzRileyLowpass<float>;
                break;
        }
        *filter = new DistanceAttenuation(lowpass);
    }
    else
    {
        *filter = new Filter<float>();
    }
    return NoErrors;
}


ErrorNum DeserializedDistance::BuildFilter(Filter<float>** filter,  PackageManager& manager, GameObject& obj)
{
    return BuildFilter(filter, manager);
}