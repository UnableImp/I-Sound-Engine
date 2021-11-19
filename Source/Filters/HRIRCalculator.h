//
// Created by zack on 11/1/21.
//

#ifndef I_SOUND_ENGINE_HRIRCALCULATOR_H
#define I_SOUND_ENGINE_HRIRCALCULATOR_H

#include "Filter.h"
#include "SoundContainer.h"
#include "AudioPackage/PackageManager.h"
#include "RealTimeParameters/GameObjectManager.h"
#include "WavContainer.h"

constexpr float pi = 3.14159265359f;

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

    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        memset(left, 0, numSamples * sizeof(float));
        memset(right, 0, numSamples * sizeof(float));

        //--------------------------------------------
        // Calculate current angle to get HRIR from
        //--------------------------------------------
        const auto& listener = GameObjectManager::GetListenerPosition();
        const auto& source = obj.GetTransform();

        IVector3 sourceDir = listener.postion - source.postion;

        // Get the angle betwen the forward vector and the source
        float listenerAngle = std::atan2(listener.forward.z,listener.forward.x);
        float sourceAngle = std::atan2(sourceDir.z, sourceDir.x);

        // -1 to flip the phase angle as kemar is counter clock wise but atan2 is clockwise
        float angle = -1 * ((listenerAngle - sourceAngle) * (180.0f / pi));
        int kemarAngle = (int)angle + (5 - ((int)angle % 5));

        // Calculate elevation
        IVector3 elevDir = listener.up - source.postion;

        //--------------------------------------------------
        // Read HRIR int buffers
        //--------------------------------------------------
        uint64_t id = static_cast<uint64_t>(currentAngle) << 32; // Angle
        id |= static_cast<uint64_t>(currentEvel) << 41; // Evelation
        id |= static_cast<uint64_t>(1) << 52; // Kemar

        assert(packageManager.GetSounds().find(id) != packageManager.GetSounds().end());

        WavContainer<float> leftIR(packageManager.GetSounds()[id]);

        id |= static_cast<uint64_t>(1) << 51; // Right ear

        assert(packageManager.GetSounds().find(id) != packageManager.GetSounds().end());

        WavContainer<float> rightIR(packageManager.GetSounds()[id]);

        leftIR.GetNextSamples(numSamples, left, left, obj);
        rightIR.GetNextSamples(numSamples, right, right, obj);

        //----------------------------------------------
        // Check if angle have changed
        //----------------------------------------------
        if(currentAngle != kemarAngle)
        {
            currentAngle = kemarAngle;

            if (currentAngle < 0)
                currentAngle += 360;
            if (currentAngle >= 360)
                currentAngle -= 360;
            return 1; // impulse is going to change
        }
        return 0; // impulse didnt change
    }

private:

    int GetNextSamplesSame(int numSamples, float* left, float* right, const GameObject& obj)
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

        leftIR.GetNextSamples(numSamples, left, left, obj);
        rightIR.GetNextSamples(numSamples, right, right, obj);

        return 0;
    }

//    int GetNextSampleMoveToGoal(int numSamples, float* left, float* right, const GameObject& obj)
//    {
//        memset(leftCurrent, 0, numSamples * sizeof(float));
//        memset(rightCurrent, 0, numSamples * sizeof(float));
//        memset(leftGoal, 0, numSamples * sizeof(float));
//        memset(rightGoal, 0, numSamples * sizeof(float));
//
//        uint64_t idCurr = currentAngle << 32; // Angle
//        idCurr |= static_cast<uint64_t>(currentEvel) << 41; // Evelation
//        idCurr |= static_cast<uint64_t>(1) << 52; // Kemar
//        assert(packageManager.GetSounds().find(idCurr) != packageManager.GetSounds().end());
//        WavContainer<float> leftCurIR(packageManager.GetSounds()[idCurr]);
//        idCurr |= static_cast<uint64_t>(1) << 51; // Right ear
//        assert(packageManager.GetSounds().find(idCurr) != packageManager.GetSounds().end());
//        WavContainer<float> rightCurIR(packageManager.GetSounds()[idCurr]);
//        leftCurIR.GetNextSamples(numSamples, leftCurrent, leftCurrent, obj);
//        rightCurIR.GetNextSamples(numSamples, rightCurrent, rightCurrent, obj);
//
//        uint64_t idGoal = goalAngle << 32; // Angle
//        idGoal |= static_cast<uint64_t>(goalEvel) << 41; // Evelation
//        idGoal |= static_cast<uint64_t>(1) << 52; // Kemar
//        assert(packageManager.GetSounds().find(idGoal) != packageManager.GetSounds().end());
//        WavContainer<float> leftGoalIR(packageManager.GetSounds()[idGoal]);
//        idGoal |= static_cast<uint64_t>(1) << 51; // Right ear
//        assert(packageManager.GetSounds().find(idGoal) != packageManager.GetSounds().end());
//        WavContainer<float> rightGoalIR(packageManager.GetSounds()[idGoal]);
//        leftCurIR.GetNextSamples(numSamples, leftGoal, leftGoal, obj);
//        rightCurIR.GetNextSamples(numSamples, rightGoal, rightGoal, obj);
//
//
//    }


    float leftCurrent[512];
    float rightCurrent[512];
    float leftGoal[512];
    float rightGoal[512];

    PackageManager& packageManager;
    int goalEvel;
    int goalAngle;
    int currentAngle;
    int  currentEvel;
};

#endif //I_SOUND_ENGINE_HRIRCALCULATOR_H
