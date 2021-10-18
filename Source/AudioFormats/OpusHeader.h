//
// Created by zack on 9/30/21.
//

#ifndef I_SOUND_ENGINE_OPUSHEADER_H
#define I_SOUND_ENGINE_OPUSHEADER_H

// Note with padding size is 20bytes but actually size is 19bytes
struct OpusHeaderChunk
{
    char signature[8];          // "opushead"
    char version;               // should always be 1
    unsigned char channels;     // can be 1-255;
    short preSkip;              // How many pcm samples to skip (little endiain) 3840 is recommended
    unsigned int sampleRate;    // Orignal input sample rate
    short outputGain;           // gain to apply to the output to get desired volume
    unsigned char mapingFamily; // output channels
};

struct MemoryMappingTable
{
    unsigned char streamCount;        // number of streams in each ogg packet
    unsigned char coupledStreamCount; //
    char* channelMapping;             // 8*channels bits
};

struct CommentHeader
{
    char signature[8];            // OpusTags
    unsigned vendorStringLength;  // Length of the vendor string
    char* vendorString;           // Vendor string not null terminated
    unsigned numOfUsercomments;   // User comments string length
    unsigned* userCommentsLength; // Length of each user comment
    char** userComments;          // User Comments
    };

// size of 27
struct OggS
{
    char signature[4]; // OggS
    char version; // 0
    char headerTypes; // Refer to docs
    char gPosition[8]; // total number of pcm frames decoded including this one
    int seirlNumber; // number to identify self
    int pageSequenceNumber; // page number
    int CheckSum; // hope you dont need this
    char pageSegments; // number of segmetns in segmation table
    char segmationTable; //
};

#endif //I_SOUND_ENGINE_OPUSHEADER_H
