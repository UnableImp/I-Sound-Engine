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
    ConvolutionFreq(int size, HRIRCalculator<float>& HRIR) : fft(size * 2), HRIR(HRIR)
    {
        currentComplex = new std::complex<float>[size* 2]();

        leftIR = new float[size* 4]();
        rightIR = new float[size* 4]();
        rightS = new float[size* 4]();
        leftS = new float[size* 4]();
        leftOverlap = new float[size * 4]();
        rightOverlap = new float[size * 4]();
        leftComplex = new  std::complex<float>[size * 4]();
        rightComplex = new  std::complex<float>[size * 4]();
    }

    virtual ~ConvolutionFreq()
    {
        delete [] currentComplex;
        delete [] leftIR;
        delete [] rightIR;
        delete [] rightS;
        delete [] leftS;
        delete [] leftOverlap;
        delete [] rightOverlap;
        delete [] leftComplex;
        delete [] rightComplex;
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right)
    {
        HRIR.GetNextSamples(numSamples * 2, leftIR, rightIR);

        //-----------------------------------------
        // Left ear
        //-----------------------------------------
        memset(leftS, 0, 2 * numSamples * sizeof(float));
        memcpy(leftS, left, numSamples * sizeof(float));


        fft.forward(leftS, leftComplex);
        fft.forward(leftIR, rightComplex);

        for(int i = 0; i < numSamples * 2; ++i)
        {
            currentComplex[i] = leftComplex[i] * rightComplex[i];
        }

        fft.inverse(currentComplex, leftS);

        //-----------------------------------------
        // Right ear
        //-----------------------------------------

        memset(rightS, 0, 2 * numSamples * sizeof(float));
        memcpy(rightS, right, numSamples * sizeof(float));


        fft.forward(rightS, leftComplex);
        fft.forward(rightIR, rightComplex);

        for(int i = 0; i < numSamples * 2; ++i)
        {
            currentComplex[i] = leftComplex[i] * rightComplex[i];
        }

        fft.inverse(currentComplex, rightS);

        //-----------------------------------------
        // Normalize
        //-----------------------------------------

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] = (leftS[i] + leftOverlap[i]) / (numSamples * 2);
            right[i] = (rightS[i]  + rightOverlap[i]) / (numSamples * 2);
        }

        memcpy(leftOverlap, leftS + numSamples, numSamples * sizeof(float));
        memcpy(rightOverlap, rightS + numSamples, numSamples * sizeof(float));

        return 0;
    }

private:

    float* leftIR;
    float* rightIR;
    float* leftS;
    float* rightS;
    float* leftOverlap;
    float* rightOverlap;
    std::complex<float>* leftComplex;
    std::complex<float>* rightComplex;
    std::complex<float>* currentComplex;

    pffft::Fft<float> fft;
    HRIRCalculator<float>& HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
