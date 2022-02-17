//
// Created by zacke on 1/29/2022.
//

#ifndef I_SOUND_ENGINE_ITD_H
#define I_SOUND_ENGINE_ITD_H
#include "Filter.h"
#include "deque"
#include <assert.h>
#include <iostream>
#include <chrono>
#include "RealTimeParameters/GameObjectManager.h"
#include "HRIRCalculator.h"
#include "Constants.h"

class ITD : public Filter<float>
{
public:
    ITD() : rightDelaySamplesOld(-1), leftDelaySamplesOld(-1) {}
    virtual ~ITD() {}
    /*!
     * Fills a buffer with audio samples, if no audio data is available zeros are filled
     * @param numSamples Number of samples to fill buffer with
     * @param buffer Buffer to fill
     * @return Number of samples filled
     */
    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {

        auto start = std::chrono::steady_clock::now();
        // Get listeners transform
        const auto& listenerTransform = GameObjectManager::GetListenerPosition();
        const auto& up = listenerTransform.up;
        const auto& forward = listenerTransform.forward;

        // Calculate left and right directions to listener
        auto rightDir = IVector3::Cross(up, forward);
        auto leftDir = IVector3{0,0,0} - rightDir;

        // Calculate were left and right ear are located
        float headRadius = GameObject::GetParam<float>("HeadRadius");

        // TODO test if normalization is needed
        auto leftEar = (leftDir * headRadius) + listenerTransform.postion;
        auto rightEar = (rightDir * headRadius) + listenerTransform.postion;

        // Get distances to each ear
        float leftEarDist = IVector3::Distance(leftEar, obj.GetPosition());
        float rightEarDist = IVector3::Distance(rightEar, obj.GetPosition());

        // Convert to sample rate delay
        float speedScaler = GameObject::GetParam<float>("DistanceScaler");
        const float speedOfSound = 343 * speedScaler; // Speed of sound?

        int leftDelaySamplesNew = (leftEarDist / speedOfSound) * sampleRate;
        int rightDelaySamplesNew = (rightEarDist / speedOfSound) * sampleRate;

         float ShouldWoodworth = GameObject::GetParam<float>("Woodworth");
        if(ShouldWoodworth)
        {
            const auto& headToObj =  (obj.GetPosition() - listenerTransform.postion).Normalized();

            auto newForward = IVector3{forward.x, 0 , forward.z};
            auto newHeadToObj = IVector3{headToObj.x, 0, headToObj.z};

            float angle = IVector3::Angle(newForward, newHeadToObj);

            if (angle > pi / 2)
                angle = pi - angle;


            float itd = (headRadius / speedOfSound) * (std::sin(angle) + angle);
            int ITDDelay = itd * sampleRate;

            if(rightDelaySamplesNew < leftDelaySamplesNew)
                leftDelaySamplesNew = rightDelaySamplesNew + ITDDelay;
            else
                rightDelaySamplesNew = leftDelaySamplesNew + ITDDelay;

        }

        if(rightDelaySamplesNew < 3)
            rightDelaySamplesNew = 3;
        if(leftDelaySamplesNew < 3)
            leftDelaySamplesNew = 3;

        if(leftDelaySamplesOld == -1)
        {
            leftDelaySamplesOld = leftDelaySamplesNew;
            rightDelaySamplesOld = rightDelaySamplesNew;
        }

        for(int i = 0; i < numSamples; ++i)
        {
            int leftDelaySamples = this->lerp(leftDelaySamplesOld, leftDelaySamplesNew, static_cast<float>(i)  / (numSamples));
            int rightDelaySamples = this->lerp(rightDelaySamplesOld, rightDelaySamplesNew, static_cast<float>(i)  / (numSamples));

            while (leftDelay.size() < leftDelaySamples + i)
                leftDelay.push_back(0);
            while (rightDelay.size() < rightDelaySamples + i)
                rightDelay.push_back(0);

            if (i + leftDelaySamples < leftDelay.size())
            {
                leftDelay[i + leftDelaySamples] += left[i];
                leftDelay[i + leftDelaySamples] /= 2;
            } else
                leftDelay.push_back(left[i]);

            if (i + rightDelaySamples < rightDelay.size())
            {
                rightDelay[i + rightDelaySamples] += right[i];
                rightDelay[i + rightDelaySamples] /= 2;
            } else
                rightDelay.push_back(right[i]);
        }

        leftDelaySamplesOld = leftDelaySamplesNew;
        rightDelaySamplesOld = rightDelaySamplesNew;



        assert(rightDelay.size() >= numSamples);
        assert(leftDelay.size() >= numSamples);

        for(int i = 0; i < numSamples; ++i)
        {
            if(leftDelay[1] == 0)
                left[i] = this->lerp(leftDelay.front(), leftDelay[2], 1/2.0f);
            else
                left[i] = leftDelay[1];

            if(rightDelay[1] == 0)
                right[i] = this->lerp(rightDelay.front(), rightDelay[2], 1/2.0f);
            else
                right[i] = rightDelay[1];

            leftDelay.pop_front();
            rightDelay.pop_front();
        }


        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<float> time = end - start;
        GameObject::SetParam("ITDLoadTemp", GameObject::GetParam<float>("ITDLoadTemp") + time.count());
        return 0;
    }


private:
    std::deque<float> leftDelay;
    std::deque<float> rightDelay;
    int leftDelaySamplesOld;
    int rightDelaySamplesOld;
};

#endif //I_SOUND_ENGINE_ITD_H
