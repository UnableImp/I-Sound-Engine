//
// Created by zack on 10/27/21.
//

#ifndef I_SOUND_ENGINE_CONVOLUTIONFREQ_H
#define I_SOUND_ENGINE_CONVOLUTIONFREQ_H

#include "Filter.h"
#include "HRIRCalculator.h"
#include "pffft.hpp"

class ConvolutionFreq : public Filter<float>
{
public:
    ConvolutionFreq(int size, HRIRCalculator<float> HRIR) : fft(size), HRIR(HRIR)
    {
        currentComplex = new std::complex<float>[size]();
        IRComplex = new std::complex<float>[size]();
        leftIR = new float[size]();
        rightIR = new float[size]();
        rightS = new float[size]();
        leftS = new float[size]();
    }

    virtual ~ConvolutionFreq()
    {
        delete [] currentComplex;
        delete [] IRComplex;
        delete [] leftIR;
        delete [] rightIR;
        delete [] rightS;
        delete [] leftS;
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right)
    {
        HRIR.GetNextSamples(numSamples, leftIR, rightIR);

        //-----------------------------------------
        // Left ear
        //-----------------------------------------
        fft.forward(left, currentComplex);
        fft.forward(leftIR, IRComplex);

        for(int i = 0; i < numSamples/2; ++i)
        {
            currentComplex[i] *= IRComplex[i];
        }

        fft.inverse(currentComplex, leftS);

        //-----------------------------------------
        // Right ear
        //-----------------------------------------
        fft.forward(right, currentComplex);
        fft.forward(rightIR, IRComplex);

//        for(int i = 0; i < numSamples; ++i)
//        {
//            currentComplex[i] *= IRComplex[i];
//        }

        fft.inverse(currentComplex, right);

        //-----------------------------------------
        // Normalize
        //-----------------------------------------

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
    float* leftS;
    float* rightS;
    std::complex<float>* currentComplex;
    std::complex<float>* IRComplex;

    pffft::Fft<float> fft;
    HRIRCalculator<float> HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
