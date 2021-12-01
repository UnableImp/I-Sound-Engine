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
        newComplexLeft = new std::complex<float>[size * 4]();
        newComplexRight = new std::complex<float>[size * 4]();
    }

    virtual ~ConvolutionFreq()
    {
        delete [] newComplexRight;
        delete [] newComplexLeft;
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

        GetHRTF(leftComplex, rightComplex, numSamples, left, right, obj);
        GetHRTF(newComplexLeft, newComplexRight, numSamples, left, right, obj);


        for(int i = 0; i < numSamples * 2; ++i)
        {
            leftComplex[i] = this->lerp(leftComplex[i], newComplexLeft[i], i / (numSamples * 2));
            rightComplex[i] = this->lerp(rightComplex[i], newComplexRight[i], i / (numSamples * 2));
        }

        fft.inverse(leftComplex, leftS);
        fft.inverse(rightComplex, rightS);

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

    void GetHRTF(std::complex<float>* leftOut, std::complex<float>* rightOut, int numSamples, float* left, float* right, const GameObject& obj)
    {
        memset(leftS, 0, 2*numSamples*sizeof(float));
        memset(rightS, 0, 2 * numSamples * sizeof(float));
        memset(leftIR, 0, 2*numSamples*sizeof(float));
        memset(rightIR, 0, 2 * numSamples * sizeof(float));

        HRIR.GetNextSamples(numSamples * 2, leftIR, rightIR, obj);

        fft.forward(leftIR, leftIRComplex);
        fft.forward(rightIR, rightIRComplex);

        //-----------------------------------------
        // Left ear
        //-----------------------------------------
        memset(leftS, 0, 2 * numSamples * sizeof(float));
        memcpy(leftS, left, numSamples * sizeof(float));

        fft.forward(leftS, leftOut);
        //fft.forward(leftIR, rightComplex);

        for(int i = 0; i < numSamples * 2; ++i)
        {
            leftOut[i] = leftOut[i] * leftIRComplex[i];
        }

        //-----------------------------------------
        // Right ear
        //-----------------------------------------

        memset(rightS, 0, 2 * numSamples * sizeof(float));
        memcpy(rightS, right, numSamples * sizeof(float));

        fft.forward(rightS, rightOut);
        //fft.forward(rightIR, rightComplex);

        for(int i = 0; i < numSamples * 2; ++i)
        {
            rightOut[i] = rightOut[i] * rightIRComplex[i];
        }
    }

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
    std::complex<float>* newComplexLeft;
    std::complex<float>* newComplexRight;

    pffft::Fft<float> fft;
    HRIRCalculator<float>& HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
