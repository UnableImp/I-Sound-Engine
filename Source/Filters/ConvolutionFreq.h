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
    ConvolutionFreq(int size) : fft(size), size(size)
    {
        bufferLeft = new float[size/2]();
        bufferRight = new float[size/2]();
        jointBuffer = new float[size]();
        lastLeftComplex= new std::complex<float>[size]();
        lastRightComplex = new std::complex<float>[size]();
        currentLeftComplex= new std::complex<float>[size]();
        currentRightComplex = new std::complex<float>[size]();
    }
    virtual int GetNextSamples(int numSamples, float* left, float* right)
    {
        // Convert to frequency space

        // copy last half of last frame for overlap
        memcpy(jointBuffer, bufferLeft, (size / 2) * sizeof(float));
        memcpy(bufferLeft, left + (size / 2), (size / 2) * sizeof(float));

        // Add in first half of current frame
        for(int i = 0; i < size / 2; ++i)
        {
            jointBuffer[i] += left[i];
        }

        // copy the first half into second half  for overlap
        memcpy(jointBuffer + (size /  2 ), left, (size / 2) * sizeof(float));

        // Add in second half of current frame
        for(int i = size / 2; i < size; ++i)
        {
            jointBuffer[i] = left[i];
        }

        // Convert to frequency space
        fft.forward(jointBuffer, currentLeftComplex);

        // Apply windows and other effects

        //convert back into time space
        fft.inverse(currentLeftComplex, left);

        return 0;
    }

private:
    float* bufferLeft;
    float* bufferRight;
    float* jointBuffer;
    std::complex<float>* lastLeftComplex;
    std::complex<float>* lastRightComplex;
    std::complex<float>* currentLeftComplex;
    std::complex<float>* currentRightComplex;
    pffft::Fft<float> fft;
    int size;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
