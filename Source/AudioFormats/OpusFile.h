//
// Created by zack on 9/29/21.
//

#ifndef I_SOUND_ENGINE_OPUSFILE_H
#define I_SOUND_ENGINE_OPUSFILE_H

#include <string>
#include <fstream>
#include "WavHeader.h"
#include "../ErrorList.h"
#include "OpusHeader.h"

class OpusFile
{
public:
    /*!
     * Ctor for wav object wrapper.
     * @param path Path to a wav file to use
     */
    explicit OpusFile(const std::string& path);
    ~OpusFile();
    /*!
     * Checks returns wether opus file is valid or not
     * @return True - valid opus file, False otherwise
     */
    operator bool() const;

    /*!
     * Retunrs the error state
     * @return Error
     */
    ErrorNum GetError() const;

    /*!
     * gets the size of buffer needed to read audio data
     * @return Total data size
     */
    //unsigned GetDataSize() const;

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

    static void WriteToFile(char* opusData, int size, OpusHeaderChunk header, char* fileName);
    static int WriteOggStoBuffer(char* buffer, OggS header);
    static int GetSegementSize(char* buffer, int& packetSize);
private:
    ErrorNum errorState;
    unsigned dataPosition;
    unsigned dataSize;
    std::fstream opusFile;
    OpusHeaderChunk opusHeader;
    CommentHeader commentHeader;

    int CRCCheckSum();
};


#endif //I_SOUND_ENGINE_OPUSFILE_H
