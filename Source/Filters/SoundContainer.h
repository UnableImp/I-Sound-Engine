/******************************************************************************/
/*!
\file   SoundContainer.h
\author Zack Krolikowksi
\date   8/22/2021

    Abstract class for all sound encodings to inhearent from
*/
/******************************************************************************/
#ifndef I_SOUND_ENGINE_SOUNDCONTAINER_H
#define I_SOUND_ENGINE_SOUNDCONTAINER_H

#include <cmath>
#include "Filter.h"

// TODO find a proper location for sample
/*!
 * Left and right samples for given index
 */


// TODO find a proper location for channel type
/*!
 * The types of channels for a given sound
 */
enum ChannelType
{
  Mono = 1,
  Stereo = 2
};

template<typename sampleType>
class SoundContainer : public Filter<sampleType>
{
public:
    /*!
     * Ctor, init playbackModifier speed to 1 so sound plays at oringal speed
     */
    SoundContainer() :
                        playbackModifier(static_cast<sampleType>(1)),
                        totalLoops(0),
                        pitchShiftCeil(0),
                        pitchShiftFloor(0),
                        pitchShiftStart(0),
                        currentLoopCount(0)
                        {}

    /*!
     * Fills a buffer with audio samples, if no audio data is available zeros are filled
     * @param numSamples Number of samples to fill buffer with
     * @param buffer Buffer to fill
     * @return Number of samples filled
     */
    virtual int GetNextSamples(int numSamples, float* left, float* right, const GameObject& obj) = 0;

    /*!
     * Reset the filter
     */
    virtual void Reset() = 0;

    /*!
     * Gets a sample from current position to offset based on play back speed
     * @param offset How far to move from current position
     * @return The sample at the position
     */
    //virtual Frame<sampleType> GetSampleFromOffset(int offset) = 0;

    /*!
     * Sets the playback speed
     *   1.0 = Original speed
     *  >1.0 = Faster play back speed
     *  <1.0 = Slower play back speed
     * @param modifier The new play back speed
     */
    void SetPlayBackSpeed(sampleType modifier)
    {
        playbackModifier = modifier;
    }

    /*!
     * Adjust the original pitch by the given cents
     * @param cents Adjustment to original pitch
     */
    void SetPitch(int cents)
    {
        pitchShiftStart = cents;
        //TODO find method that doesnt use pow
        playbackModifier = static_cast<sampleType>(std::pow(static_cast<sampleType>(2),
                                                            static_cast<sampleType>(cents) / 1200));
    }

    void RandomPitch()
    {
        int range = abs(pitchShiftCeil) + abs(pitchShiftFloor);

        if(range == 0)
            return;

        int newPitch = rand() % range;

        newPitch -= pitchShiftFloor;
        newPitch += pitchShiftStart;

        playbackModifier = static_cast<sampleType>(std::pow(static_cast<sampleType>(2),
                                                            static_cast<sampleType>(newPitch) / 1200));
    }

    /*!
     * Sets the valid range in cents that a sound can vary by
     * @param ceil Max value
     * @param floor Min value
     */
    void SetRandomPitchRange(int ceil, int floor)
    {
        pitchShiftCeil = ceil;
        pitchShiftFloor = floor;
    }

    /*!
     * Number of loops to loop by
     * @param loops -1 = nonstop looping
     */
    void SetLoopCount(int loops)
    {
        totalLoops = loops;
    }

    /*!
     * Sets the volume of a single source
     * @param newVolume volume in range [0-1]
     */
    void SetVolume(float newVolume)
    {
        volume = newVolume;
    }

    /*!
     * Set the position to seek to
     * @param position position to seek to
     */
    //virtual void Seek(int position)                    = 0;
    //virtural const storageType& GetAllSamples()      = 0;

protected:
    void FillZeros(int count,  float* left, float* right)
    {
        for(int i = 0; i < count; ++i)
        {
            left[i] = 0;
            right[i] = 0;
        }
    }



    /*!
     * The playback modifier to control speed of sound playback
     */
    sampleType playbackModifier;
    float volume;
    int totalLoops;
    int currentLoopCount;
    int pitchShiftCeil;
    int pitchShiftFloor;
    int pitchShiftStart;
};

#endif //I_SOUND_ENGINE_SOUNDCONTAINER_H
