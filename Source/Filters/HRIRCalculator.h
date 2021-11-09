//
// Created by zack on 11/1/21.
//

#ifndef I_SOUND_ENGINE_HRIRCALCULATOR_H
#define I_SOUND_ENGINE_HRIRCALCULATOR_H

#include "Filter.h"
#include "SoundContainer.h"
#include "AudioPackage/PackageManager.h"

template<typename sampleType>
class HRIRCalculator : public Filter<sampleType>
{
public:
    HRIRCalculator(/*listener ref, object ref,*/PackageManager& packageManager ) : packageManager(packageManager),
                                                                                   angle(0),
                                                                                   evel(0)
    {}

    virtual ~HRIRCalculator() {}

    virtual int GetNextSamples(int numSamples, float* left, float* right) override
    {

        memset(left, 0, numSamples * sizeof(float));
        memset(right, 0, numSamples * sizeof(float));

        uint64_t id = angle << 32; // Angle
        id |= static_cast<uint64_t>(evel) << 41; // Evelation
        id |= static_cast<uint64_t>(1) << 52; // Kemar

        assert(packageManager.GetSounds().find(id) != packageManager.GetSounds().end());

        WavContainer<float> leftIR(packageManager.GetSounds()[id]);

         id |= static_cast<uint64_t>(1) << 51; // Right ear


        assert(packageManager.GetSounds().find(id) != packageManager.GetSounds().end());

        WavContainer<float> rightIR(packageManager.GetSounds()[id]);

        leftIR.GetNextSamples(numSamples, left, left);
        rightIR.GetNextSamples(numSamples, right, right);

        return 0;
    }

    void SetAngle(uint64_t newAngle)
    {
        angle = newAngle;
    }

    void SetElev(uint64_t newElev)
    {
        evel = newElev;
    }

private:
    PackageManager& packageManager;
    uint64_t evel;
    uint64_t angle;
};

#endif //I_SOUND_ENGINE_HRIRCALCULATOR_H
