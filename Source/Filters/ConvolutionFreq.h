//
// Created by zack on 10/27/21.
//

#ifndef I_SOUND_ENGINE_CONVOLUTIONFREQ_H
#define I_SOUND_ENGINE_CONVOLUTIONFREQ_H

#include "Filter.h"
#include "HRIRCalculator.h"
#include "pffft.hpp"
#include <deque>

constexpr int Overlap = 128;
constexpr int BlockSize = 512;

class ConvolutionFreq : public Filter<float>
{
public:
    ConvolutionFreq(int size, HRIRCalculator<float>& HRIR) : fft(BlockSize * 2), HRIR(HRIR), leftOverlap((BlockSize * 2) - Overlap), rightOverlap((BlockSize * 2) - Overlap)
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
        for(int offset = 0; offset < BlockSize; offset += Overlap)
        {
            CalculateBlock(Overlap, left + offset, right + offset, obj);
        }
        return 0;
    }

private:

    void CalculateBlock(int numSamples, float* left, float* right, const GameObject& obj)
    {
        HRIR.GetNextSamples(BlockSize * 2, leftIR, rightIR, obj);

        //-----------------------------------------
        // Left ear
        //-----------------------------------------
        memset(leftS, 0, 2 * BlockSize * sizeof(float));
        memcpy(leftS, left, numSamples * sizeof(float));


        fft.forward(leftS, leftComplex);
        fft.forward(leftIR, rightComplex);

        for(int i = 0; i < BlockSize * 2; ++i)
        {
            currentComplex[i] = leftComplex[i] * rightComplex[i];
        }

        fft.inverse(currentComplex, leftS);

        //-----------------------------------------
        // Right ear
        //-----------------------------------------

        memset(rightS, 0, 2 * BlockSize * sizeof(float));
        memcpy(rightS, right, numSamples * sizeof(float));


        fft.forward(rightS, leftComplex);
        fft.forward(rightIR, rightComplex);

        for(int i = 0; i < BlockSize * 2; ++i)
        {
            currentComplex[i] = leftComplex[i] * rightComplex[i];
        }

        fft.inverse(currentComplex, rightS);

        //-----------------------------------------
        // Normalize
        //-----------------------------------------

        for(int i = 0; i < numSamples; ++i)
        {
            left[i] = (leftS[i] + leftOverlap.front()) / (numSamples * 2);
            right[i] = (rightS[i]  + rightOverlap.front()) / (numSamples * 2);
            left[i] *= (static_cast<float>(Overlap) / BlockSize) * 0.8f;
            right[i] *= (static_cast<float>(Overlap) / BlockSize) * 0.8f;

            leftOverlap.pop_front();
            rightOverlap.pop_front();
        }

        for(int i = Overlap, j = 0; i < BlockSize; ++i, ++j)
        {
            leftOverlap[j] = leftS[i];
            rightOverlap[j] = rightS[i];
        }

        for(int i = (BlockSize * 2) - Overlap; i < BlockSize * 2; ++i)
        {
            leftOverlap.push_back(leftS[i]);
            rightOverlap.push_back(rightS[i]);
        }
    }

    float* leftIR;
    float* rightIR;
    float* leftS;
    float* rightS;
    //float* leftOverlap;
    //float* rightOverlap;
    std::complex<float>* leftComplex;
    std::complex<float>* rightComplex;
    std::complex<float>* currentComplex;


    std::deque<float> leftOverlap;
    std::deque<float> rightOverlap;

    pffft::Fft<float> fft;
    HRIRCalculator<float>& HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
