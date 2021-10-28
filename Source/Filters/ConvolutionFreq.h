//
// Created by zack on 10/27/21.
//

#ifndef I_SOUND_ENGINE_CONVOLUTIONFREQ_H
#define I_SOUND_ENGINE_CONVOLUTIONFREQ_H

#include "Filter.h"
#include "pffft.hpp"

class ConvolutionFreq : public Filter<float>
{
public:
    virtual int GetNextSamples(int numSamples, float* left, float* right)
    {
        // Convert to frequnce space
        //auto freqLeft = fft.spectrumVector();
        //auto freqRight = fft.spectrumVector();

        std::complex<float> freqLeft[1024];
        std::complex<float> freqRight[1024];
        fft.forward(left, freqLeft);
        fft.forward(right, freqRight);

        // Temp to test fft
        memset(left, 0, sizeof(float) * numSamples);
        memset(right, 0, sizeof (float) * numSamples);

        // Convert back to time domain
        fft.inverse(freqLeft, left);
        fft.inverse(freqRight, right);
return 0;
    }

private:
    pffft::Fft<float> fft {1024};
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
