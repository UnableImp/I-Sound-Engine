//
// Created by zacke on 1/29/2022.
//

#ifndef I_SOUND_ENGINE_ITD_H
#define I_SOUND_ENGINE_ITD_H
#include "Filter.h"
#include "deque"
#include <assert.h>
#include <iostream>

constexpr int sampleRate = 44100;

class ITD : public Filter<float>
{
public:
    virtual ~ITD() {}
    /*!
     * Fills a buffer with audio samples, if no audio data is available zeros are filled
     * @param numSamples Number of samples to fill buffer with
     * @param buffer Buffer to fill
     * @return Number of samples filled
     */
    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        // Get listeners transform
        const auto& listenerTransform = GameObjectManager::GetListenerPosition();
        const auto& up = listenerTransform.up;
        const auto& forward = listenerTransform.forward;

        // Calculate left and right directions to listener
        auto leftDir = IVector3::Cross(up.Normalized(), forward.Normalized());
        auto rightDir = IVector3{0,0,0} - leftDir;

        // Calculate were left and right ear are located
        float headRadius = GameObject::GetParam<float>("HeadRadius");

        // TODO test if nomraliztion is needed
        auto leftEar = (leftDir * headRadius) + listenerTransform.postion;
        auto rightEar = (rightDir * headRadius) + listenerTransform.postion;

        // Get distances to each ear
        float leftEarDist = IVector3::Distance(leftEar, obj.GetPosition());
        float rightEarDist = IVector3::Distance(rightEar, obj.GetPosition());

        // Convert to sample rate delay
        float speedScaler = GameObject::GetParam<float>("DistanceScaler");
        const float speedOfSound = 343 * speedScaler; // Speed of sound?

        int leftDelaySamples = (leftEarDist / speedOfSound) * sampleRate;
        int rightDelaySamples = (rightEarDist / speedOfSound) * sampleRate;

        //std::cout << "Left: " << leftDelaySamples << " Right: " << rightDelaySamples << std::endl;

        while(leftDelay.size() < leftDelaySamples)
            leftDelay.push_back(0);
        while(rightDelay.size() < rightDelaySamples)
            rightDelay.push_back(0);

        for(int i = 0; i < numSamples; ++i)
        {
            if(i + leftDelaySamples < leftDelay.size())
                leftDelay[i + leftDelaySamples] += left[i];
            else
                leftDelay.push_back(left[i]);
        }

        for(int i = 0; i < numSamples; ++i)
        {
            if(i + rightDelaySamples < rightDelay.size())
                rightDelay[i + rightDelaySamples] += right[i];
            else
                rightDelay.push_back(right[i]);
        }

        assert(rightDelay.size() >= numSamples);
        assert(leftDelay.size() >= numSamples);

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] = leftDelay.front();
            right[i] = rightDelay.front();
            leftDelay.pop_front();
            rightDelay.pop_front();
        }
        return 0;
    }


private:
    std::deque<float> leftDelay;
    std::deque<float> rightDelay;
};

#endif //I_SOUND_ENGINE_ITD_H
