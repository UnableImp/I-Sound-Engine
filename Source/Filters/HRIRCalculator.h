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
constexpr float delta = 0.003f;
constexpr int blockSize = 512;

template<typename sampleType>
class HRIRCalculator : public Filter<sampleType>
{
public:
    HRIRCalculator(/*listener ref, object ref,*/PackageManager& packageManager ) : packageManager(packageManager),
                                                                                   currentEvel(0),
                                                                                   currentAngle(0),
                                                                                   step(-1)
    {}

    virtual ~HRIRCalculator() {}

    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        memset(left, 0, numSamples * sizeof(float));
        memset(right, 0, numSamples * sizeof(float));

        const auto& listener = GameObjectManager::GetListenerPosition();
        const auto& source = obj.GetTransform();

        IVector3 sourceDir = source.postion - listener.postion;

        // Get the angle betwen the forward vector and the source
        float listenerAngle = std::atan2(listener.forward.z,listener.forward.x);
        float sourceAngle = std::atan2(sourceDir.z, sourceDir.x);

        if(listenerAngle < 0)
            listenerAngle += 2 * pi;
        if(sourceAngle < 0)
            sourceAngle += 2 * pi;

        float angle = ((listenerAngle - sourceAngle) * (180.0f / pi));

        int shouldUsePreprocessing = static_cast<int>(std::any_cast<float>(obj.GetParam("Preprocess")));
        if(shouldUsePreprocessing)
            currentAngle = static_cast<int>(angle);
        else
            currentAngle = (int)angle + (5 - ((int)angle %  5));

        if(currentAngle < 0)
            currentAngle += 360;
        if(currentAngle >= 360)
            currentAngle -= 360;


        int shouldLerp = static_cast<int>(std::any_cast<float>(obj.GetParam("LerpHRIR")));
        if(shouldLerp)
        {
            if(step = -1)
            {
                step = 1;
                oldAngle = currentAngle;
            }

            float overlapSize = std::any_cast<float>(obj.GetParam("Overlap"));
            currentAngle = this->lerp(oldAngle, currentAngle, overlapSize/blockSize);

            ++step;
            if(((overlapSize/blockSize) * step) + 0.01 > 1.0f)
            {
                step = 1;
                oldAngle = currentAngle;
            }
        }


        // Calculate elevation
        IVector3 elevDir = listener.up - source.postion;

        //float ListenerEvel = std::atan2(lis)

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

        float phaseAlign = std::any_cast<float>(obj.GetParam("PhaseAlign"));
        if(phaseAlign > 0)
        {

            int offset = 0;

            //-----------------------------------
            // Left ear
            //-----------------------------------
            for(int i = 0; i < blockSize; ++i)
            {
                if(std::abs(left[i]) > delta)
                {
                    offset = i;
                    break;
                }
            }
            for(int i = offset, j = 0; i < blockSize * 2; ++i, ++j)
            {
                std::swap(left[i], left[j]);
            }

            //-----------------------------------
            // Right ear
            //-----------------------------------
            for(int i = 0; i < blockSize; ++i)
            {
                if(std::abs(right[i]) > delta)
                {
                    offset = i;
                    break;
                }
            }
            for(int i = offset, j = 0; i < blockSize * 2; ++i, ++j)
            {
                std::swap(right[i], right[j]);
            }
        }

        return 0;
    }

private:

    float leftCurrent[512];
    float rightCurrent[512];
    float leftGoal[512];
    float rightGoal[512];

    PackageManager& packageManager;
    int currentAngle;
    int  currentEvel;
    int step;
    int oldAngle;
};

#endif //I_SOUND_ENGINE_HRIRCALCULATOR_H
