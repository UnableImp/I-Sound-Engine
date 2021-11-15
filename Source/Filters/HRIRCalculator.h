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
                                                                                   currentEvel(0),
                                                                                   currentAngle(0),
                                                                                   goalAngle(0),
                                                                                   goalEvel(0)
    {}

    virtual ~HRIRCalculator() {}

    virtual int GetNextSamples(int numSamples, float* left, float* right)
    {
        currentAngle = goalAngle;
        currentEvel = goalEvel;

        if(currentAngle == goalAngle && currentEvel == goalEvel)
        {
            return GetNextSamplesSame(numSamples, left, right);
        }
        else
        {
            return GetNextSamplesSame(numSamples, left, right);
        }
    }

    void SetAngle(uint64_t newAngle)
    {
        goalAngle = newAngle;
    }

    void SetElev(uint64_t newElev)
    {
        goalEvel = newElev;
    }

private:

    int GetNextSamplesSame(int numSamples, float* left, float* right)
    {

        memset(left, 0, numSamples * sizeof(float));
        memset(right, 0, numSamples * sizeof(float));

        uint64_t id = currentAngle << 32; // Angle
        id |= static_cast<uint64_t>(currentEvel) << 41; // Evelation
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

    int GetNextSampleMoveToGoal(int numSamples, float* left, float* right)
    {
        memset(leftCurrent, 0, numSamples * sizeof(float));
        memset(rightCurrent, 0, numSamples * sizeof(float));
        memset(leftGoal, 0, numSamples * sizeof(float));
        memset(rightGoal, 0, numSamples * sizeof(float));

        uint64_t idCurr = currentAngle << 32; // Angle
        idCurr |= static_cast<uint64_t>(currentEvel) << 41; // Evelation
        idCurr |= static_cast<uint64_t>(1) << 52; // Kemar
        assert(packageManager.GetSounds().find(idCurr) != packageManager.GetSounds().end());
        WavContainer<float> leftCurIR(packageManager.GetSounds()[idCurr]);
        idCurr |= static_cast<uint64_t>(1) << 51; // Right ear
        assert(packageManager.GetSounds().find(idCurr) != packageManager.GetSounds().end());
        WavContainer<float> rightCurIR(packageManager.GetSounds()[idCurr]);
        leftCurIR.GetNextSamples(numSamples, leftCurrent, leftCurrent);
        rightCurIR.GetNextSamples(numSamples, rightCurrent, rightCurrent);

        uint64_t idGoal = goalAngle << 32; // Angle
        idGoal |= static_cast<uint64_t>(goalEvel) << 41; // Evelation
        idGoal |= static_cast<uint64_t>(1) << 52; // Kemar
        assert(packageManager.GetSounds().find(idGoal) != packageManager.GetSounds().end());
        WavContainer<float> leftGoalIR(packageManager.GetSounds()[idGoal]);
        idGoal |= static_cast<uint64_t>(1) << 51; // Right ear
        assert(packageManager.GetSounds().find(idGoal) != packageManager.GetSounds().end());
        WavContainer<float> rightGoalIR(packageManager.GetSounds()[idGoal]);
        leftCurIR.GetNextSamples(numSamples, leftGoal, leftGoal);
        rightCurIR.GetNextSamples(numSamples, rightGoal, rightGoal);


    }


    float leftCurrent[512];
    float rightCurrent[512];
    float leftGoal[512];
    float rightGoal[512];

    PackageManager& packageManager;
    uint64_t goalEvel;
    uint64_t goalAngle;
    uint64_t currentAngle;
    uint64_t  currentEvel;
};

#endif //I_SOUND_ENGINE_HRIRCALCULATOR_H
