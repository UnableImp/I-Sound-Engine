//
// Created by zacke on 8/31/2021.
//

#ifndef I_SOUND_ENGINE_WAVFILE_H
#define I_SOUND_ENGINE_WAVFILE_H

#include <string>
#include <fstream>
#include "WavHeader.h"
#include "../ErrorList.h"

class WavFile
{
public:
    /*!
     * Ctor for wav object wrapper.
     * @param path Path to a wav file to use
     */
    explicit WavFile(const std::string& path);

    /*!
     * Checks returns wether wav file is valid or not
     * @return True - valid wav file, False otherwise
     */
    operator bool() const;

    /*!
     * Retunrs the error state
     * @return Error
     */
    ErrorNum GetError() const;

    /*!
     * Get the format chunk of the wav file
     * @return Format chunk
     */
    const FormatHeader& GetFormat() const;

    /*!
     * gets the size of buffer needed to read audio data
     * @return Total data size
     */
    unsigned GetDataSize() const;

    /*!
     * Fills given buffer with data, make sure buffer is big enough to store all samples
     * @param buffer buffer to fill
     * @return true - read was successful
     */
    bool GetDataInNativeType(char* buffer);

    /*!
     * Fills buffer with samples in range [-1, 1]
     * @param buffer buffer to be filled
     * @return true - read was successful
     */
    bool GetDataAsFloat(float* buffer);

private:
    ErrorNum errorState;
    unsigned dataPosition;
    unsigned dataSize;
    std::fstream wavFile;
    FormatHeader fmtHeader;
};

#endif //I_SOUND_ENGINE_WAVFILE_H