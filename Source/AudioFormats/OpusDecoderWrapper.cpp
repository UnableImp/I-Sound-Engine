//
// Created by zack on 9/29/21.
//

#include "OpusDecoderWrapper.h"


OpusDecoderWrapper::OpusDecoderWrapper(int sampleRate, int channels)
{
    //   /* Create a new decoder state. */
    //   decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
    decoder = opus_decoder_create(sampleRate, channels, &error);
}

int OpusDecoderWrapper::Decode(char *input, int inputSize, short *output, int frameSize)
{
    int value = opus_decode(decoder, reinterpret_cast<const unsigned char *>(input), inputSize, output, frameSize, 0);
    return value;
}

int OpusDecoderWrapper::DecodeFloat(char *input, int inputSize, float *output, int frameSize)
{
    int value = opus_decode_float(decoder, reinterpret_cast<const unsigned char *>(input), inputSize, output, frameSize, 0);
    return value;
}

OpusDecoderWrapper::~OpusDecoderWrapper()
{
    opus_decoder_destroy(decoder);
}