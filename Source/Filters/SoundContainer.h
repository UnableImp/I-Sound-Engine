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
                        playbackModifier(static_cast<sampleType>(1)) {}

    /*!
     * Fills a buffer with audio samples, if no audio data is available zeros are filled
     * @param numSamples Number of samples to fill buffer with
     * @param buffer Buffer to fill
     * @return Number of samples filled
     */
    virtual int GetNextSamples(int numSamples, float* left, float* right) = 0;

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
    void setPlayBackSpeed(sampleType modifier)
    {
        playbackModifier = modifier;
    }

    /*!
     * Adjust the original pitch by the given cents
     * @param cents Adjustment to original pitch
     */
    void setPitch(int cents)
    {
        //TODO find method that doesnt use pow
        playbackModifier = static_cast<sampleType>(std::pow(static_cast<sampleType>(2),
                                                            static_cast<sampleType>(cents) / 1200));
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
};

#endif //I_SOUND_ENGINE_SOUNDCONTAINER_H
