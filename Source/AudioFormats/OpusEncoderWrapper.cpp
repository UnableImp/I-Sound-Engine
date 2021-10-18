//
// Created by zack on 9/30/21.
//

#include "OpusEncoderWrapper.h"

//constexpr int sampleRate = 48000;
constexpr int application = OPUS_APPLICATION_AUDIO;

OpusEncoderWrapper::OpusEncoderWrapper(int samplingRate, int channels)
{
    encoder = opus_encoder_create(samplingRate, channels, application, &error);
    //opus_encoder_ctl(encoder, OPUS_SET_BITRATE(512000));
}

int OpusEncoderWrapper::GetOpusError() const
{
    return error;
}

int OpusEncoderWrapper::Encode(short* pcmSamples, int pcmFrames, char* opusData, int opusDataMaxSize)
{
   int value = opus_encode(encoder, pcmSamples, pcmFrames, reinterpret_cast<unsigned char*>(opusData), opusDataMaxSize);


   if(value < 0)
       error = value;
   return value;
}

int OpusEncoderWrapper::Encode(float* inBuff, int inSamples, char* outBuff, int outSize)
{
    int value = opus_encode_float(encoder, inBuff, inSamples, reinterpret_cast<unsigned char*>(outBuff), outSize);
    if(value < 0)
        error = value;
    return value;
}

OpusEncoderWrapper::~OpusEncoderWrapper()
{
    opus_encoder_destroy(encoder);
}