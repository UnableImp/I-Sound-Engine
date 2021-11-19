//
// Created by zack on 10/21/21.
//

#ifndef I_SOUND_ENGINE_REALTIMEAUDIO_H
#define I_SOUND_ENGINE_REALTIMEAUDIO_H

#include "Events/EventManager.h"

#include "portaudio.h"

class RealTimeAudio
{
public:
    RealTimeAudio(EventManager&);
    ~RealTimeAudio();
    void Start();
    void Stop();

    void GenerateAudioData(int numSamples, float* buffer);

private:


    PaStreamParameters m_output_params; // Params
    PaStream *m_stream;                 // Stream
    bool m_IsInit;                      // is pa init

    EventManager& eventManager;

    Frame<float> dataBuffer[512];
    int index;

};


#endif //I_SOUND_ENGINE_REALTIMEAUDIO_H
