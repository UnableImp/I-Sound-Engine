//
// Created by zacke on 8/31/2021.
//

#include "WavFile.h"
#include <fstream>
#include "OpusEncoderWrapper.h"
#include "OpusHeader.h"
#include <cstring>
#include "OpusFile.h"

#include <limits>

constexpr int EncodeSize = 960;

WavFile::WavFile(const std::string& path) : errorState(ErrorNum::NoErrors),
                                     wavFile(path, std::ios_base::binary | std::ios_base::in),
                                     fmtHeader()
{
    // Open file
    if(!wavFile)
    {
        errorState = ErrorNum::FailedToFindFile;
        return;
    }

    // Read the wav header chunk
    char header[sizeof(RiffHeader)];
    wavFile.read(header, sizeof(RiffHeader));
    RiffHeader riffHeader = *reinterpret_cast<RiffHeader*>(header);

    // Check if is a riff file
    if(riffHeader.riff_label[0] != 'R' ||
       riffHeader.riff_label[1] != 'I' ||
       riffHeader.riff_label[2] != 'F' ||
       riffHeader.riff_label[3] != 'F')
    {
        errorState = ErrorNum::NotAPCMFile;
        return;
    }

    // Check if is a wav file
    if(riffHeader.file_tag[0] != 'W' ||
       riffHeader.file_tag[1] != 'A' ||
       riffHeader.file_tag[2] != 'V' ||
       riffHeader.file_tag[3] != 'E')
    {
        errorState = ErrorNum::NotAPCMFile;
        return;
    }

    // Find Format Chunk
    wavFile.read(reinterpret_cast<char*>(&fmtHeader), sizeof(GenericHeaderChunk));
    while(wavFile && (fmtHeader.label[0] != 'f' ||
                      fmtHeader.label[1] != 'm' ||
                      fmtHeader.label[2] != 't' ||
                      fmtHeader.label[3] != ' ' ))
    {
        wavFile.seekg(fmtHeader.size, std::ios_base::cur);
        wavFile.read(reinterpret_cast<char*>(&fmtHeader), sizeof(GenericHeaderChunk));
    }
    if(!wavFile)
    {
        errorState = ErrorNum::NotAPCMFile;
        return;
    }

    //Read fmt chunk and verify
    wavFile.read(reinterpret_cast<char*>(&fmtHeader) + sizeof(GenericHeaderChunk),
                 sizeof(FormatHeader) - sizeof(GenericHeaderChunk));

    if(fmtHeader.audio_format != 1)
    {
        errorState = ErrorNum::UnsportedFile;
        return;
    }

//    if(fmtHeader.bytes_per_sample * 8 != fmtHeader.bits_per_sample)
//    {
//        errorState = ErrorNum::PossibleCorution;
//        return;
//    }

    // Find data Chunk
    GenericHeaderChunk chunk;
    wavFile.read(reinterpret_cast<char*>(&chunk), sizeof(GenericHeaderChunk));
    while(wavFile && (chunk.label[0] != 'd' ||
                      chunk.label[1] != 'a' ||
                      chunk.label[2] != 't' ||
                      chunk.label[3] != 'a' ))
    {
        wavFile.seekg(chunk.chunkSize, std::ios_base::cur);
        wavFile.read(reinterpret_cast<char*>(&chunk), sizeof(GenericHeaderChunk));
    }

    dataPosition = wavFile.tellg();
    dataSize = chunk.chunkSize;
}

WavFile::operator bool() const
{
    if(errorState == ErrorNum::NoErrors)
        return true;
    return false;
}

ErrorNum WavFile::GetError() const
{
    return errorState;
}

const FormatHeader& WavFile::GetFormat() const
{

    return fmtHeader;
}

unsigned WavFile::GetDataSize() const
{
    return dataSize;
}

bool WavFile::GetDataInNativeType(char* buffer)
{
    wavFile.seekg(dataPosition, std::ios_base::beg);
    wavFile.read(buffer, dataSize);
    return true;
}

bool WavFile::GetDataAsFloat(float *buffer)
{
    /* Idea
     * float - smallest decmal is atleast 4 bytes
     * short - 2 bytes
     * Read data samples into second half of the buffer then
     * start to convert samples at the start
     */

    int numSamples = dataSize / (fmtHeader.bits_per_sample / 8);

                                // buffer size                       native data size
    unsigned readPointer  = (sizeof(float) * numSamples) - (numSamples * (fmtHeader.bits_per_sample / 8));  // in number of bytes
    unsigned writePos     = 0; // first index

    GetDataInNativeType(reinterpret_cast<char*>(buffer) + readPointer);
    //wavFile.read(reinterpret_cast<char*>(buffer) + readPointer, dataSize * fmtHeader.bytes_per_sample);

    switch(fmtHeader.bits_per_sample)
    {
        // 8 bit conversion
        case 8:
            for(unsigned i = 0; i < dataSize; ++i)
            {

                float v = *(reinterpret_cast<unsigned char*>(buffer) + readPointer);
                      v -= 128;
                      v /= 128;
                buffer[writePos] = v;/*((*(reinterpret_cast<char*>(buffer) + readPointer))
                                    - std::numeric_limits<char>::max())
                                    / std::numeric_limits<char>::max();*/
                ++writePos;
                ++readPointer;
            }
            return true;
        // 16 bit conversion
        case 16:
            int loopSize = dataSize / (fmtHeader.bits_per_sample / 8);
            for(unsigned i = 0; i < loopSize; ++i)
            {
                buffer[writePos] = (*reinterpret_cast<short*>(reinterpret_cast<char *>(buffer) + (readPointer)))
                                    / static_cast<float>(std::numeric_limits<short>::max());

                ++writePos;
                readPointer += 2;
            }
            return true;
    }
    return false;
}

int WavFile::GetDataAsOpus(char* buffer)
{
    // TODO Convert other types to 16 bit for opus, could also use floats
    GetDataInNativeType(buffer);

    // Three buffers per ogg grouping
    char writeBuffer[EncodeSize];

    OggS header {'O','g','g','S'};
    header.version = 0;
    memset(header.gPosition, 0, 8);
    header.pageSegments = 1;
    header.seirlNumber = 101;
    header.CheckSum = 0;
    header.headerTypes = 0x0;
    header.pageSequenceNumber = 2;

    OpusEncoderWrapper encoder(48000, fmtHeader.channel_count);

    //TODO Have more than one chunk per OggS
    int offset = 0;
    int readOffset = 0;
    while(readOffset <= dataSize - (EncodeSize * 2))
    {
        int size = encoder.Encode(reinterpret_cast<short*>(buffer + readOffset), EncodeSize, writeBuffer, EncodeSize * 2);
        readOffset += EncodeSize * 4;
        if(size < 0)
        {
            errorState = ErrorNum::OpusError;
            return -1;
        }

        offset += OpusFile::WriteOggStoBuffer(buffer + offset, header);

        int writeSize = size;
        while(writeSize > 0)
        {
            if(writeSize > 255) // Buffer doesnt fit in a char
            {
                *reinterpret_cast<unsigned char*>(buffer + offset) = static_cast<unsigned char>(255);
                writeSize -= 255;
            }
            else if (writeSize == 255) // fits exactly into one char
            {
                *reinterpret_cast<unsigned char*>(buffer + offset) = static_cast<unsigned char>(255);
                ++offset;
                *(buffer + offset) = 0;
                writeSize = 0;
            }
            else // size fits in an unsigned char
            {
                *reinterpret_cast<unsigned char*>(buffer + offset) = static_cast<unsigned char>(writeSize);
                writeSize = 0;
            }
            ++offset;
        }
        memcpy(buffer+offset, writeBuffer, size);
        offset += size;

        ++header.pageSequenceNumber;
    }
    return offset;
}