//
// Created by zack on 10/27/21.
//

#ifndef I_SOUND_ENGINE_CONVOLUTIONFREQ_H
#define I_SOUND_ENGINE_CONVOLUTIONFREQ_H

#include "Filter.h"
#include "HRIRCalculator.h"
#include "pffft.hpp"
#include <deque>
#include <chrono>
#include "RingBuffer.h"
#include "immintrin.h"

constexpr int BlockSize = 512;

class ConvolutionFreq : public Filter<float>
{
public:
    ConvolutionFreq(int size, HRIRCalculator<float>& HRIR) : fft(BlockSize * 2),
    HRIR(HRIR),
    Overlap(static_cast<int>((GameObject::GetParam<float>("Overlap"))))
    {
        currentComplex = new std::complex<float>[size* 2]();

        leftIR = new float[size*2]();
        rightIR = new float[size*2]();
        rightS = new float[size*2]();
        leftS = new float[size*2]();
        leftComplex = new std::complex<float>[size*2]();
        rightComplex = new std::complex<float>[size*2]();
        leftOld = new float[size*2]();
        rightOld = new float[size*2]();
        leftOverlap = new float[size * 2]();
        rightOverlap = new float[size * 2]();
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
        delete [] rightOld;
        delete [] leftOld;
        delete [] leftOverlap;
        delete [] rightOverlap;
    }

    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj)
    {
        auto start = std::chrono::steady_clock::now();

        float crossFade = (obj.GetParam<float>("CrossFade"));
        if(crossFade != 0.0f)
        {
            memcpy(leftOld, left, sizeof(float) * numSamples);
            memcpy(rightOld, right, sizeof(float) * numSamples);
//            auto overlap1 = leftOverlap;
//            auto overlap2 = rightOverlap;

            GetNextSamplesFromBuffer(numSamples, leftOld, rightOld, obj, false);
            HRIR.GetNextSamples(BlockSize * 2, leftIR, rightIR, obj);
            GetNextSamplesFromBuffer(numSamples, left, right, obj, true);

            for(int i = 0; i < numSamples; i++)
            {
                // incorrect crossfade
                //left[i] = lerp(leftOld[i], left[i], static_cast<float>(i)/numSamples);
                //right[i] = lerp(rightOld[i], right[i], static_cast<float>(i)/numSamples);

                // linear crossfade
                left[i]  = ((1 - static_cast<float>(i) / numSamples) * leftOld[i]) +
                           ((static_cast<float>(i) / numSamples)     * left[i]);
                right[i] = ((1 - static_cast<float>(i) / numSamples) * rightOld[i]) +
                           ((static_cast<float>(i) / numSamples)     * right[i]);

                // Easing fade - sounds way worse than linear??
//                left[i] =  (this->EaseInQuart(1.0f - (static_cast<float>(i) / numSamples)) * leftOld[i]) +
//                           (this->EaseInQuart( static_cast<float>(i) / numSamples) * left[i]);
//                right[i] = (this->EaseInQuart(1.0f - (static_cast<float>(i) / numSamples)) * rightOld[i]) +
//                           (this->EaseInQuart( static_cast<float>(i) / numSamples) * right[i]);
            }
        }
        else
        {
            HRIR.GetNextSamples(BlockSize * 2, leftIR, rightIR, obj);
            GetNextSamplesFromBuffer(numSamples, left, right, obj, true);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<float> time = end - start;
        GameObject::SetParam("HRTFLoadTemp", GameObject::GetParam<float>("HRTFLoadTemp") + time.count());
        return 0;
    }

private:

    int GetNextSamplesFromBuffer(int numSamples, float* left, float* right, const GameObject& obj, bool saveOverlap)
    {
        for(int offset = 0; offset < BlockSize; offset += Overlap)
        {
            CalculateBlock(Overlap, left + offset, right + offset, obj, saveOverlap);
        }
        return 0;
    }

    void CalculateBlock(int numSamples, float* left, float* right, const GameObject& obj, bool saveOverlap)
    {
        //-----------------------------------------
        // Left ear
        //-----------------------------------------
        memset(leftS, 0,  2* BlockSize * sizeof(float));
        memset(rightS, 0,  2* BlockSize * sizeof(float));
        //memcpy(leftS, left, numSamples * sizeof(float));

        for(int i = 0; i < numSamples; ++i)
        {
            for(int j = 0; j < 512; j += 8)
            {
//                __m256 inVec{left[i],left[i],left[i],left[i],left[i],left[i],left[i],left[i]};
//                __m256 leftIRVec = _mm256_loadu_ps(&leftIR[j]);
//                __m256 rightIRVec = _mm256_loadu_ps(&rightIR[j]);
//
//                __m256 leftOut  = _mm256_mul_ps(inVec, leftIRVec);
//                __m256 rightOut = _mm256_mul_ps(inVec, rightIRVec);
//
//                inVec = _mm256_loadu_ps(&leftS[i+j]);
//                __m256 outVec = _mm256_add_ps(inVec, leftOut);
//                _mm256_storeu_ps(&leftS[i+j], outVec);
//
//                inVec  = _mm256_loadu_ps(&rightS[i+j]);
//                outVec = _mm256_add_ps(inVec, rightOut);
//                _mm256_storeu_ps(&rightS[i+j], outVec);

                leftS[i+j] += leftIR[j] * left[i];
                rightS[i+j] += rightIR[j] * left[i];
            }
        }

//        fft.forwardToInternalLayout(leftS, reinterpret_cast<float *>(leftComplex));
//        fft.forwardToInternalLayout(leftIR, reinterpret_cast<float *>(rightComplex));
//
//        fft.convolve(reinterpret_cast<const float *>(leftComplex), reinterpret_cast<const float *>(rightComplex),
//                     reinterpret_cast<float *>(currentComplex), 1.0f/ (numSamples * 2));
//
//        fft.inverseFromInternalLayout(reinterpret_cast<const float *>(currentComplex), leftS);
//
//        //-----------------------------------------
//        // Right ear
//        //-----------------------------------------
//
//        fft.forwardToInternalLayout(rightIR, reinterpret_cast<float *>(rightComplex));
//
//        fft.convolve(reinterpret_cast<const float *>(leftComplex), reinterpret_cast<const float *>(rightComplex),
//                     reinterpret_cast<float *>(currentComplex), 0.5f/ (numSamples * 2));
//
//        fft.inverseFromInternalLayout(reinterpret_cast<const float *>(currentComplex), rightS);

        //-----------------------------------------
        // Normalize
        //-----------------------------------------

        for(int i = 0; i < 512; i += 8)
        {
            __m256 one = _mm256_loadu_ps(&left[i]);
            __m256 two = _mm256_loadu_ps(&leftOverlap[i]);

            __m256 three = _mm256_loadu_ps(&right[i]);
            __m256 four = _mm256_loadu_ps(&rightOverlap[i]);

            __m256 out1 = _mm256_add_ps(one, two);
            __m256 out2 = _mm256_add_ps(three, four);

            _mm256_storeu_ps(&left[i], out1);
            _mm256_storeu_ps(&right[i], out2);
        }

//        for(int i = 0; i < numSamples; ++i)
//        {
//            left[i] = (leftS[i] + leftOverlap[i]);
//            right[i] = (rightS[i]  + rightOverlap[i]);
//        }

        if(saveOverlap)
        {
//            for (int i = Overlap, j = 0; j < leftOverlapT.size(); ++i, ++j)
//            {
//                leftOverlapT[j] += leftS[i];
//                rightOverlapT[j] += rightS[i];
//            }

            memcpy(leftOverlap, &leftS[512], 512 * sizeof(float));
            memcpy(rightOverlap, &rightS[512], 512 * sizeof(float));
        }
    }

    const int Overlap;

    float* leftIR;
    float* rightIR;
    float* leftS;
    float* rightS;

    float* leftOld;
    float* rightOld;

    //float* leftOverlap;
    //float* rightOverlap;
    std::complex<float>* leftComplex;
    std::complex<float>* rightComplex;
    std::complex<float>* currentComplex;


    float* leftOverlap;
    float* rightOverlap;

    pffft::Fft<float> fft;
    HRIRCalculator<float>& HRIR;
};

#endif //I_SOUND_ENGINE_CONVOLUTIONFREQ_H
