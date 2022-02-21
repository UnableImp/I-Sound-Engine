//
// Created by zacke on 2/20/2022.
//

#ifndef I_SOUND_ENGINE_DISTANCEATTENUATION_H
#define I_SOUND_ENGINE_DISTANCEATTENUATION_H
#include "Filter.h"
#include "Biqaud/Biquad.h"

class DistanceAttenuation : public Filter<float>
{
public:
    DistanceAttenuation(Biquad<float>* lowpass) : lowpass(lowpass) {}

    ~DistanceAttenuation()
    {
        if(lowpass)
            delete lowpass;
    }

    int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj) override
    {

        const auto& listener = GameObjectManager::GetListenerPosition();
        const auto& source = obj.GetTransform();

        float distScaler = std::max(IVector3::Distance(listener.postion, source.postion) / obj.GetParam<float>("MaxSoundDistance"), 0.0f);

        float levelScaler = 1.0f;
        switch(static_cast<int>(obj.GetParam<float>("RolloffFunc")))
        {
            case 0:
                levelScaler = this->EaseOutQuart(1-distScaler);
                break;
            case 1:
                levelScaler = this->EaseOutCubic(1-distScaler);
                break;
            case 2:
                levelScaler = this->EaseOutQuad(1-distScaler);
                break;
            case 3:
                levelScaler = 1-distScaler;
                break;
            case 4:
                levelScaler = this->EaseInQuad(1-distScaler);
                break;
            case 5:
                levelScaler = this->EaseInCubic(1-distScaler);
                break;
            case 6:
                levelScaler = this->EaseInQuart(1-distScaler);
                break;
            default:
                assert(!"Not a valid rolloff fucntion");
        }
        if(lowpass)
        {
            lowpass->SetCutoff((levelScaler) * (sampleRate / 2));
            lowpass->GetNextSamples(numSamples, left, right, obj);
        }
        for(int i = 0; i < numSamples; ++i)
        {
            left[i] *= levelScaler;
        }

        return 0;
    }

private:
    Biquad<float>* lowpass;
};

#endif //I_SOUND_ENGINE_DISTANCEATTENUATION_H
