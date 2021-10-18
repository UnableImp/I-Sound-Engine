//
// Created by zack on 9/29/21.
//

#ifndef I_SOUND_ENGINE_OPUSDECODERWRAPPER_H
#define I_SOUND_ENGINE_OPUSDECODERWRAPPER_H

#include "opus.h"

class OpusDecoderWrapper
{
public:
    //TODO How to get channel count???
    explicit OpusDecoderWrapper(int sampleRate, int channels);
    ~OpusDecoderWrapper();

    int Decode(char *input, int inputSize, short *output, int frameSize);
    int DecodeFloat(char *input, int inputSize, float *output, int frameSize);

private:
    OpusDecoder* decoder = nullptr;
    int error = 0;
};


#endif //I_SOUND_ENGINE_OPUSDECODERWRAPPER_H
