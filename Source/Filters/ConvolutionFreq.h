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
    ConvolutionFreq(int size) : fft(size)
    {

        currentComplex = new std::complex<float>[size]();
        leftIR = new float[size]();
        rightIR = new float[size]();
    }

    virtual ~ConvolutionFreq()
    {
        delete [] currentComplex;
        delete [] leftIR;
        delete [] rightIR;
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right)
    {

        fft.forward(left, currentComplex);
        fft.inverse(currentComplex, left);

        fft.forward(right, currentComplex);
        fft.inverse(currentComplex, right);

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] /= numSamples;
            right[i] /= numSamples;
        }

        return 0;
    }

private:

    float* leftIR;
    float* rightIR;
    std::complex<float>* currentComplex;
    pffft::Fft<float> fft;

};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
