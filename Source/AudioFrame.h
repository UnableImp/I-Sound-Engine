//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_AUDIOFRAME_H
#define I_SOUND_ENGINE_AUDIOFRAME_H
template <typename sampleType>
struct Frame
{
    sampleType leftChannel;  //! Sample for left channel
    sampleType rightChannel; //! Sample for right Channel
    Frame<sampleType>& operator+=(Frame<sampleType> rhs)
    {
        leftChannel += rhs.leftChannel;
        rightChannel += rhs.rightChannel;

        return *this;
    }

    Frame<sampleType> operator/(int rhs)
    {
        Frame<sampleType> value;
        value.leftChannel = leftChannel / rhs;
        value.rightChannel = rightChannel / rhs;
        return value;
    }

    Frame<sampleType>& operator/=(Frame<sampleType>& rhs)
    {
        leftChannel /= rhs.leftChannel;
        rightChannel /= rhs.rightChannel;
        return *this;
    }

    Frame<sampleType>& operator/=(int rhs)
    {
        leftChannel /= rhs;
        rightChannel /= rhs;
        return *this;
    }

};
#endif //I_SOUND_ENGINE_AUDIOFRAME_H
