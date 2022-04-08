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
#include "RingDeque.h"

class ITD : public Filter<float>
{
public:
    ITD() : rightDelaySamplesOld(-1), leftDelaySamplesOld(-1), leftDelay(1<<17), rightDelay(1<<17) {}
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
        float headRadius = obj.GetParam<float>("HeadRadius");

        // TODO test if normalization is needed
        auto leftEar = (leftDir * headRadius) + listenerTransform.postion;
        auto rightEar = (rightDir * headRadius) + listenerTransform.postion;

        // Get distances to each ear
        float leftEarDist = IVector3::Distance(leftEar, obj.GetPosition());
        float rightEarDist = IVector3::Distance(rightEar, obj.GetPosition());

        // Convert to sample rate delay
        float speedScaler = obj.GetParam<float>("DistanceScaler");
        const float speedOfSound = 343; // Speed of sound?

        float leftDelaySamplesNew = (leftEarDist / speedOfSound) * sampleRate * speedScaler;
        float rightDelaySamplesNew = (rightEarDist / speedOfSound) * sampleRate * speedScaler;

         float ShouldWoodworth = obj.GetParam<float>("Woodworth");
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
            leftLast = 0;
            rightLast = 0;

            leftOffset = 512.0;
            rightOffset = 512.0;

            leftVelLast = 0;
            rightVelLast = 0;

            for(int i = 0; i < leftDelaySamplesNew; ++i)
            {
                leftDelay.push_back(0);
            }
            for(int i = 0; i < rightDelaySamplesNew; ++i)
            {
                rightDelay.push_back(0);
            }
        }

        leftOffset -= 512.0;
        rightOffset -= 512.0;

        float leftVel = ((static_cast<float>(leftDelaySamplesOld - leftDelaySamplesNew) / 512) * 343) / 2.0f;
        float rightVel = ((static_cast<float>(rightDelaySamplesOld - rightDelaySamplesNew) / 512) * 343) / 2.0f;

        leftVel += leftVelLast;
        leftVel /= 2.0f;
        rightVel += rightVelLast;
        rightVel /= 2.0f;

        float leftStep = (343 + leftVel) / 343;
        float rightStep =(343 + rightVel) / 343;

        // Left Delay
        while(leftOffset < numSamples - 1)
        {
            float value = 0;
            float offset = leftOffset - static_cast<int>(leftOffset);
            if (leftOffset < 0)
            {
                value = this->lerp(leftLast, left[0], offset);
            } else
            {
                value = this->lerp(left[static_cast<int>(leftOffset)], left[static_cast<int>(leftOffset) + 1], offset);
            }
            leftDelay.push_back(value);

            leftOffset += leftStep;
        }

        while(rightOffset < numSamples - 1)
        {
            float value = 0;
            float offset = rightOffset - static_cast<int>(rightOffset);
            if (rightOffset < 0)
            {
                value = this->lerp(rightLast, right[0], offset);
            } else
            {
                value = this->lerp(right[static_cast<int>(rightOffset)], right[static_cast<int>(rightOffset) + 1], offset);
            }
            rightDelay.push_back(value);

            rightOffset += rightStep;
        }

        leftLast = left[numSamples - 1];
        rightLast = right[numSamples - 1];

        leftDelaySamplesOld = leftDelaySamplesNew;
        rightDelaySamplesOld = rightDelaySamplesNew;
        leftVelLast = leftVel;
        rightVelLast = rightVel;

        assert(rightDelay.size() >= numSamples);
        assert(leftDelay.size() >= numSamples);

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] = leftDelay.front();
            right[i] = rightDelay.front();

            leftDelay.pop_front();
            rightDelay.pop_front();
        }


        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<float> time = end - start;
        GameObject::SetParamStatic("ITDLoadTemp", GameObject::GetParamStatic<float>("ITDLoadTemp") + time.count());
        return 0;
    }


private:
    RingDeque<float> leftDelay;
    RingDeque<float> rightDelay;
    float leftDelaySamplesOld;
    float rightDelaySamplesOld;

    float leftLast;
    float rightLast;

    float leftVelLast;
    float rightVelLast;

    double leftOffset;
    double rightOffset;
};

#endif //I_SOUND_ENGINE_ITD_H
