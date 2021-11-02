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
    ConvolutionFreq(int size, HRIRCalculator<float> HRIR) : fft(size * 2), HRIR(HRIR)
    {
        currentComplex = new std::complex<float>[size* 2]();

        leftIR = new float[size* 4]();
        rightIR = new float[size* 4]();
        rightS = new float[size* 4]();
        leftS = new float[size* 4]();
        leftComplex = new float[size * 4]();
        rightComplex = new float[size * 4]();
    }

    virtual ~ConvolutionFreq()
    {
        delete [] currentComplex;
        delete [] leftIR;
        delete [] rightIR;
        delete [] rightS;
        delete [] leftS;
        delete [] leftComplex;
        delete [] rightComplex;
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right)
    {
        HRIR.GetNextSamples(numSamples, leftIR, rightIR);

        //-----------------------------------------
        // Left ear
        //-----------------------------------------
        memset(leftS, 0, 2 *numSamples * sizeof(float));
        memcpy(leftS, left, numSamples * sizeof(float));

        //pffft::AlignedVector<float> vector = fft.internalLayoutVector();

        fft.forwardToInternalLayout(leftS, leftComplex);
        fft.forwardToInternalLayout(leftIR, rightComplex);

        fft.convolve(leftComplex, rightComplex, leftS, 1);

        fft.reorderSpectrum(leftS, currentComplex);

        fft.inverse(currentComplex, leftS);

        //-----------------------------------------
        // Right ear
        //-----------------------------------------
        memset(rightS, 0, 2 *numSamples * sizeof(float));
        memcpy(rightS, right, numSamples * sizeof(float));

        fft.forwardToInternalLayout(rightS, rightComplex);
        fft.forwardToInternalLayout(rightIR, leftComplex);

        fft.convolve(leftComplex, rightComplex, rightS, 1);

        fft.reorderSpectrum(rightS, currentComplex);

        fft.inverse(currentComplex, rightS);

        //-----------------------------------------
        // Normalize
        //-----------------------------------------

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] = leftS[i] / (numSamples * numSamples);
            right[i] = rightS[i] / (numSamples * numSamples * 16);
        }

        return 0;
    }

private:

    float* leftIR;
    float* rightIR;
    float* leftS;
    float* rightS;
    float* leftComplex;
    float* rightComplex;
    std::complex<float>* currentComplex;

    pffft::Fft<float> fft;
    HRIRCalculator<float> HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
