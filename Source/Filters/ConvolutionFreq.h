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
        leftIRComplex = new std::complex<float>[size * 4]();
        rightIRComplex = new std::complex<float>[size * 4]();
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
        delete [] leftIRComplex;
        delete [] rightIRComplex;
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        memset(leftS, 0, 2*numSamples*sizeof(float));
        memset(rightS, 0, 2 * numSamples * sizeof(float));
        memset(leftIR, 0, 2*numSamples*sizeof(float));
        memset(rightIR, 0, 2 * numSamples * sizeof(float));

        HRIR.GetNextSamples(numSamples * 2, leftIR, rightIR, obj);
        HRIR.GetNextSamples(numSamples * 2, leftS, rightS, obj);

        fft.forward(leftIR, leftComplex);
        fft.forward(rightIR, rightComplex);
        fft.forward(leftS, leftIRComplex);
        fft.forward(rightS, rightIRComplex);

//        for(int i = 0; i < numSamples * 2; ++i)
//        {
//            //std::cout << leftIRComplex[i] << " ";
//            leftIRComplex[i] = this->lerp(leftComplex[i], leftIRComplex[i], (i+512) / (numSamples * 2));
//            rightIRComplex[i] = this->lerp(rightComplex[i], rightIRComplex[i], (i + 512) / (numSamples * 2));
//            //std::cout << leftIRComplex[i] << " " << i << std::endl;
//        }

        //-----------------------------------------
        // Left ear
        //-----------------------------------------
        memset(leftS, 0, 2 * numSamples * sizeof(float));
        memcpy(leftS, left, numSamples * sizeof(float));

        fft.forward(leftS, leftComplex);
        //fft.forward(leftIR, rightComplex);

        for(int i = 0; i < numSamples * 2; ++i)
        {
            currentComplex[i] = leftComplex[i] * leftIRComplex[i];
        }

        fft.inverse(currentComplex, leftS);

        //-----------------------------------------
        // Right ear
        //-----------------------------------------

        memset(rightS, 0, 2 * numSamples * sizeof(float));
        memcpy(rightS, right, numSamples * sizeof(float));

        fft.forward(rightS, rightComplex);
        //fft.forward(rightIR, rightComplex);

        for(int i = 0; i < numSamples * 2; ++i)
        {
            currentComplex[i] = rightComplex[i] * rightIRComplex[i];
        }

        fft.inverse(currentComplex, rightS);

        //-----------------------------------------
        // Normalize
        //-----------------------------------------

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] = ((leftS[i] + leftOverlap[i]) / (numSamples * 2)) / 2;
            right[i] = ((rightS[i]  + rightOverlap[i]) / (numSamples * 2)) / 2;
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
    std::complex<float>* leftIRComplex;
    std::complex<float>* rightIRComplex;
    std::complex<float>* currentComplex;

    pffft::Fft<float> fft;
    HRIRCalculator<float>& HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
