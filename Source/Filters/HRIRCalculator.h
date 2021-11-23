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
#include <math.h>
#include "pffft.hpp"

constexpr float pi = 3.14159265359f;

template<typename sampleType>
class HRIRCalculator : public Filter<sampleType>
{
public:
    HRIRCalculator(/*listener ref, object ref,*/PackageManager& packageManager ) : packageManager(packageManager),
                                                                                   currentEvel(0),
                                                                                   currentAngle(0)
    {
        angle1L = new float[1024]();
        angle2L = new float[1024]();
        angle1R = new float [1024]();
        angle2R = new float [1024]();
        trash = new float[1024]();
    }

    virtual ~HRIRCalculator()
    {
        delete [] angle2L;
        delete [] angle1L;
        delete [] angle2R;
        delete [] angle1R;
        delete trash;
    }

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

        float angle = (listenerAngle - sourceAngle) * (180.0f / pi);

        if(angle < 0)
            angle += 360;

        //int KEMARAngle = (int)angle + (5 - ((int)angle % 5));

        // Calculate elevation
        IVector3 elevDir = listener.up - source.postion;

        int KEMARup = (int)angle + (5 - ((int)angle % 5));
        int KEMARdown = KEMARup - 5;

        memset(angle1R, 0, numSamples * sizeof(float));
        memset(angle1L, 0, numSamples * sizeof(float));
        memset(angle2R, 0, numSamples * sizeof(float));
        memset(angle2L, 0, numSamples * sizeof(float));

        GetAngle(angle1L, angle1R, KEMARdown, numSamples, obj);
        GetAngle(angle2L, angle2R, KEMARup, numSamples, obj);

        memcpy(left, angle1L, sizeof(float) * numSamples);
        memcpy(right, angle1R, sizeof(float) * numSamples);

        return 0;
    }

private:

    float GetAngle(float* bufferl, float* bufferr, int angle, int numSamples, const GameObject& obj)
    {
        // Ensure that angle is in KEMAR data pack
        assert (angle % 5 == 0);

        if(angle == 360)
            angle = 0;

        uint64_t id = static_cast<uint64_t>(angle) << 32; // Angle
        id |= static_cast<uint64_t>(currentEvel) << 41; // Evelation
        id |= static_cast<uint64_t>(1) << 52; // Kemar

        // Make sure sound is loaded
        assert(packageManager.GetSounds().find(id) != packageManager.GetSounds().end());

        WavContainer<float> leftIR(packageManager.GetSounds()[id]);

        id |= static_cast<uint64_t>(1) << 51; // Right ear

        assert(packageManager.GetSounds().find(id) != packageManager.GetSounds().end());

        WavContainer<float> rightIR(packageManager.GetSounds()[id]);

        leftIR.GetNextSamples(numSamples, bufferl, trash, obj);
        rightIR.GetNextSamples(numSamples, bufferr, trash, obj);

        return 0;
    }

    float* angle1L;
    float* angle1R;
    float* angle2L;
    float* angle2R;

    float* trash;

    std::complex<float> leftComplex;
    std::complex<float> rightComplex;

    PackageManager& packageManager;
    int currentAngle;
    int  currentEvel;
};

#endif //I_SOUND_ENGINE_HRIRCALCULATOR_H
