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

constexpr float frameLength = 512.0f / 44100.0f;

class ITD : public Filter<float>
{
public:
    ITD() : rightDelaySamplesOld(-1), leftDelaySamplesOld(-1), leftDistOld(-1), rightDistOld(-1), leftDelay(1<<17), rightDelay(1<<17) {}
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
        const float speedOfSound = 343 * speedScaler; // Speed of sound?



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

            if(rightEarDist < leftEarDist)
                leftEarDist = rightEarDist + ITDDelay;
            else
                rightEarDist = leftEarDist + ITDDelay;

        }

        int leftDelaySamplesNew = (leftEarDist / speedOfSound) * sampleRate;
        int rightDelaySamplesNew = (rightEarDist / speedOfSound) * sampleRate;

        if(leftDistOld == -1)
        {
            leftDistOld = leftEarDist;
            rightDistOld = rightEarDist;
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

        if(leftDelay.size() == 0)
        {
            for(int i = 0; i < leftDelaySamplesNew; ++i)
                leftDelay.push_back(0);
            for(int i = 0; i < rightDelaySamplesNew; ++i)
                rightDelay.push_back(0);
        }

        float velLeft = std::min(static_cast<float>((leftEarDist - leftDistOld)) / frameLength, speedOfSound);
        float stepLeft = speedOfSound / (speedOfSound + velLeft);

        float velRight = std::min(static_cast<float>((rightEarDist - rightDistOld)) / frameLength, speedOfSound);
        float stepRight = speedOfSound / (speedOfSound + velRight);

        rightDistOld = rightEarDist;
        leftDistOld = leftEarDist;

//        if(velRight != velLeft)
//        {
//            //std::cout << "Left: " << velLeft << " Right: " << velRight;
//            std::cout << "Left: " << stepLeft << ", " << velLeft << " Right: " << stepRight << ", " << velRight << std::endl;
//        }
//        else
//        {
//            std::cout << "good" << std::endl;
//        }

        float step = 1.0f;
        while(step < static_cast<float>(numSamples))
        {
            int lower = static_cast<int>(step);
            int upper = lower + 1;
            float diff = step - static_cast<float>(lower);
            leftDelay.push_back(this->lerp(left[lower], left[upper],diff));
            step += stepLeft;
        }

        step = 1.0f;
        while(step < static_cast<float>(numSamples))
        {
            int lower = static_cast<int>(step);
            int upper = lower + 1;
            float diff = step - static_cast<float>(lower);
            rightDelay.push_back(this->lerp(right[lower], right[upper],diff));
            step += stepRight;
        }


        leftDelaySamplesOld = leftDelaySamplesNew;
        rightDelaySamplesOld = rightDelaySamplesNew;



        //assert(rightDelay.size() >= numSamples);
        //assert(leftDelay.size() >= numSamples);

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
    int leftDelaySamplesOld;
    int rightDelaySamplesOld;
    float leftDistOld;
    float rightDistOld;

};

#endif //I_SOUND_ENGINE_ITD_H
