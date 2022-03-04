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
#include "../Constants.h"


template<typename sampleType>
class HRIRCalculator : public Filter<sampleType>
{
public:
    HRIRCalculator(/*listener ref, object ref,*/PackageManager& packageManager ) : packageManager(packageManager),
                                                                                   currentEvel(0),
                                                                                   currentAngle(0),
                                                                                   step(-1),
                                                                                   collection(static_cast<int>(GameObject::GetParam<float>("HRIRSet")))
    {}

    virtual ~HRIRCalculator() {}

    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        memset(left, 0, numSamples * sizeof(float));
        memset(right, 0, numSamples * sizeof(float));

        const auto& listener = GameObjectManager::GetListenerPosition();
        const auto& source = obj.GetTransform();

        IVector3 sourceDir = source.postion - listener.postion;
        IVector3 parralleDir = IVector3{sourceDir.x, listener.forward.y, sourceDir.z};

        float elevation = IVector3::Angle(parralleDir, sourceDir);
        // Basic not scalable elevation check
        if(source.postion.y < listener.postion.y)
            elevation *= -1;

        currentEvel = static_cast<int>((elevation * 180 / pi));
        if(currentEvel < -40)
            currentEvel = -40;
        if(currentEvel < 0)
            currentEvel += 360;

        //std::cout << elevation << " " << (elevation * 180) / pi << std::endl;


        // Get the angle betwen the forward vector and the source
        float listenerAngle = std::atan2(listener.forward.z,listener.forward.x);
        float sourceAngle = std::atan2(sourceDir.z, sourceDir.x);

        if(listenerAngle < 0)
            listenerAngle += 2 * pi;
        if(sourceAngle < 0)
            sourceAngle += 2 * pi;

        float angle = ((listenerAngle - sourceAngle) * (180.0f / pi));

        int shouldUsePreprocessing = static_cast<int>((obj.GetParam<float>("Preprocess")));
        if(shouldUsePreprocessing)
            currentAngle = static_cast<int>(angle);
        else
            currentAngle = (int)angle + (5 - ((int)angle %  5));

        if(currentAngle < 0)
            currentAngle += 360;
        if(currentAngle >= 360)
            currentAngle -= 360;


        // Calculate elevation
        IVector3 elevDir = listener.up - source.postion;
        float phaseAlign = (obj.GetParam<float>("PhaseAlign"));

        //float ListenerEvel = std::atan2(lis)

        uint64_t id = static_cast<uint64_t>(currentAngle) << 32; // Angle
        id |= static_cast<uint64_t>(currentEvel) << 41; // Evelation
        id |= static_cast<uint64_t>(collection) << 52; // pack to use

        if(phaseAlign > 0.1f)
            id |= static_cast<uint64_t>(1) << 55;

        assert(packageManager.GetSounds().find(id) != packageManager.GetSounds().end());

        WavContainer<float> leftIR(packageManager.GetSounds()[id]);

        id |= static_cast<uint64_t>(1) << 51; // Right ear

        assert(packageManager.GetSounds().find(id) != packageManager.GetSounds().end());

        WavContainer<float> rightIR(packageManager.GetSounds()[id]);

        leftIR.GetNextSamples(numSamples/2, left, left, obj);
        rightIR.GetNextSamples(numSamples/2, right, right, obj);

        return 0;
    }

private:
    PackageManager& packageManager;
    int currentAngle;
    int currentEvel;
    int step;
    int collection;

};

#endif //I_SOUND_ENGINE_HRIRCALCULATOR_H
