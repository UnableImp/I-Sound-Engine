//
// Created by zack on 9/30/21.
//

#ifndef I_SOUND_ENGINE_OPUSENCODERWRAPPER_H
#define I_SOUND_ENGINE_OPUSENCODERWRAPPER_H

#include "opus.h"

class OpusEncoderWrapper
{
public:
    OpusEncoderWrapper(int samplingRate, int channels);

    int GetOpusError() const;

    /*!
     * Encodes 16bit pcm to opus format
     * @param inBuff shorts to convert
     * @param inSize number of shorts to convert, 960 is recommeded
     * @param outBuff output of encoding
     * @param outSize Max about of bytes outBuff can store
     * @return >0 is the number of bytes outBuff used, <0 is for a failure
     */
    int Encode(short* inBuff, int inSize, char* outBuff, int outSize);

    int Encode(float* inBuff, int inSamples, char* outBuff, int outSize);

private:
    OpusEncoder* encoder;
    int error = 0;
};


#endif //I_SOUND_ENGINE_OPUSENCODERWRAPPER_H
