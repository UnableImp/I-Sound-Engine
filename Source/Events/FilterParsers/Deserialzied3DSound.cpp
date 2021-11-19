//
// Created by zacke on 11/17/2021.
//

#include "Deserialized3DSound.h"
#include "Filters/ConvolutionFreq.h"

Deserialize3DSound::Deserialize3DSound(rapidjson::Value &object)
{}

ErrorNum Deserialize3DSound::BuildFilter(Filter<float> **filter,  PackageManager& manager)
{
    HRIRCalculator<float>* hrir = new HRIRCalculator<float>(manager);

    *filter = new ConvolutionFreq(512, *hrir);
    return NoErrors;
}