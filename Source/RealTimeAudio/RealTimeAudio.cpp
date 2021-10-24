//
// Created by zack on 10/21/21.
//

#include "RealTimeAudio.h"

/**
 * @brief
 *  Port audio call back function
 *
 * @param input not used
 * @param output Writing frames
 * @param frameCount Number of frames to write
 * @param timeInfo not used
 * @param statusFlags not used
 * @param userData custom class to get data samples
 * @return paContinue will always be returned
 */
int FillBuffer(const void *input, void *output, unsigned long frameCount,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags, void* userData)
{
    RealTimeAudio* audio = reinterpret_cast<RealTimeAudio*>(userData);
    float* buffer = reinterpret_cast<float*>(output);
    audio->GenerateAudioData(frameCount, buffer);
    return paContinue;
}

RealTimeAudio::RealTimeAudio(EventManager& eventManager) : eventManager(eventManager)
{
    Pa_Initialize();

    m_IsInit = true;

    // output stream parameters
    m_output_params.device = Pa_GetDefaultOutputDevice();
    if(m_output_params.device == -1)
        return;
    m_output_params.channelCount = 2;
    m_output_params.sampleFormat = paFloat32;
    m_output_params.suggestedLatency = 2*Pa_GetDeviceInfo(m_output_params.device)->defaultLowOutputLatency;
    m_output_params.hostApiSpecificStreamInfo = 0;
}

RealTimeAudio::~RealTimeAudio()
{
    if(m_IsInit)
        Stop();
}

void RealTimeAudio::Start()
{
    if(m_output_params.device == -1)
        return;
    Pa_OpenStream(&m_stream,0,&m_output_params,44100,0,paClipOff,&FillBuffer,this);
    Pa_StartStream(m_stream);
}

void RealTimeAudio::Stop()
{
    m_IsInit = false;

    Pa_StopStream(m_stream);
    Pa_CloseStream(m_stream);
    Pa_Terminate();
}

void RealTimeAudio::GenerateAudioData(int numSamples, float* buffer)
{
    eventManager.GetSamplesFromAllEvents(numSamples, reinterpret_cast<Frame<float>*>(buffer));
}