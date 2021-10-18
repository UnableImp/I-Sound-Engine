//
// Created by zacke on 8/31/2021.
//

#ifndef I_SOUND_ENGINE_WAVHEADER_H
#define I_SOUND_ENGINE_WAVHEADER_H

/*!
 * Generic and Data headers are the same header
 */
struct GenericHeaderChunk
{
    char label[4];      // label for given chunk
    unsigned chunkSize; // size of given chunk
};

/*!
 * Header for a riff file
 */
struct RiffHeader
{
    char riff_label[4]; // (00) = {'R','I','F','F'}
    unsigned riff_size; // (04) = 36 + data_size
    char file_tag[4]; // (08) = {'W','A','V','E'}
};

/*!
 * Format chunk
 */
struct FormatHeader
{
    char label[4]; // (12) = {'f','m','t',' '}
    unsigned size; // (16) = 16
    unsigned short audio_format; // (20) = 1
    unsigned short channel_count; // (22) = 1 or 2
    unsigned sampling_rate; // (24)
    unsigned bytes_per_second; // (28)
    unsigned short bytes_per_sample; //
    unsigned short bits_per_sample; // (34) = 8 or 16
};

// Minimal header data req for wav file
struct WavHeader
{
    RiffHeader riffHeader;
    FormatHeader formatHeader;
    GenericHeaderChunk dataHeader;
};

#endif //I_SOUND_ENGINE_WAVHEADER_H
