//
// Created by zack on 10/27/21.
//

#ifndef I_SOUND_ENGINE_CONVOLUTIONFREQ_H
#define I_SOUND_ENGINE_CONVOLUTIONFREQ_H

#include "Filter.h"
#include "HRIRCalculator.h"
#include "pffft.hpp"
#include <deque>

class ConvolutionFreq : public Filter<float>
{
public:
    ConvolutionFreq(int size, HRIRCalculator<float>& HRIR) : fft(size * 2), HRIR(HRIR), leftOverlap(768), rightOverlap(896)
    {
        currentComplex = new std::complex<float>[size* 2]();

        leftIR = new float[size* 4]();
        rightIR = new float[size* 4]();
        rightS = new float[size* 4]();
        leftS = new float[size* 4]();
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
        delete [] leftComplex;
        delete [] rightComplex;
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        GetNextSamplesRunner(numSamples/4, left, right, obj);
        GetNextSamplesRunner(numSamples/4, left + ((numSamples/4) * 1), right + ((numSamples/4) * 1), obj);
        GetNextSamplesRunner(numSamples/4, left + ((numSamples/4 ) * 2), right + ((numSamples/4) * 2), obj);
        GetNextSamplesRunner(numSamples/4, left + ((numSamples/4) * 3), right + ((numSamples/4) * 3), obj);
        return 0;
    }

private:
    int GetNextSamplesRunner(int numSamples, float* left, float* right, const GameObject& obj)
    {
        HRIR.GetNextSamples(1024, leftIR, rightIR, obj);

        //-----------------------------------------
        // Left ear
        //-----------------------------------------
        memset(leftS, 0, 2 * 512 * sizeof(float));
        memcpy(leftS, left, numSamples * sizeof(float));


        fft.forward(leftS, leftComplex);
        fft.forward(leftIR, rightComplex);

        for(int i = 0; i < 1024; ++i)
        {
            currentComplex[i] = leftComplex[i] * rightComplex[i];
        }

        fft.inverse(currentComplex, leftS);

        //-----------------------------------------
        // Right ear
        //-----------------------------------------

        memset(rightS, 0, 2 * 512 * sizeof(float));
        memcpy(rightS, right, numSamples * sizeof(float));


        fft.forward(rightS, leftComplex);
        fft.forward(rightIR, rightComplex);

        for(int i = 0; i < 1024; ++i)
        {
            currentComplex[i] = leftComplex[i] * rightComplex[i];
        }

        fft.inverse(currentComplex, rightS);

        //-----------------------------------------
        // Normalize
        //-----------------------------------------

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] = (leftS[i] + leftOverlap[0]) / (512 * 2);
            right[i] = (rightS[i]  + rightOverlap[0]) / (512 * 2);
            leftOverlap.pop_front();
            rightOverlap.pop_front();
        }

        for(int i = 0; i < numSamples*6; ++i)
        {
            leftOverlap[i] += leftS[i + numSamples];
            rightOverlap[i] += rightS[i +numSamples];
        }

        for(int i = 0; i < numSamples; ++i)
        {
            leftOverlap.push_back(leftS[i + numSamples*6]);
            rightOverlap.push_back(rightS[i + numSamples*6]);
        }


        return 0;
    }

    float* leftIR;
    float* rightIR;
    float* leftS;
    float* rightS;
    std::deque<float> leftOverlap;
    std::deque<float> rightOverlap;
    //float* leftOverlap;
    //float* rightOverlap;
    std::complex<float>* leftComplex;
    std::complex<float>* rightComplex;
    std::complex<float>* currentComplex;

    pffft::Fft<float> fft;
    HRIRCalculator<float>& HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
