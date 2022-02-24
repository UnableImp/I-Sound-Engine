#include "PackageEncoder.h"
#include <fstream>
#include <memory>
#include <cstdint>
#include "AudioFormats/OpusEncoderWrapper.h"
#include "AudioFormats/OpusHeader.h"

ErrorNum PackageEncoder::AddFile(WavFile& wav, uint64_t id, Encoding format)
{
    if(wav.GetError() == NoErrors)
    {
        filesToEncode.emplace_back(FileInfo{wav.GetPath(), id, format});
        int sampleCount = wav.GetDataSize() / (wav.GetFormat().bits_per_sample / 8);
        bufferSize += (sampleCount * sizeof (float)) + sizeof(uint64_t) + sizeof(WavHeader); // data size + id + header
    }
    return wav.GetError();
}

ErrorNum PackageEncoder::AddFile(const std::string &path, uint64_t id, Encoding format)
{
    WavFile wav(path);
    if(wav.GetError() == NoErrors)
    {
        filesToEncode.emplace_back(FileInfo{wav.GetPath(), id, format});
        int sampleCount = wav.GetDataSize() / (wav.GetFormat().bits_per_sample / 8);
        bufferSize += (sampleCount * sizeof (float)) + sizeof(uint64_t) + sizeof(WavHeader); // data size + id + header
    }
    return wav.GetError();
}

#include <iostream>
ErrorNum PackageEncoder::WritePackage(std::string path)
{
    //TODO get correct buffer size

    // Ensure bank can be made before doing work
    std::fstream bank(path.c_str(), std::ios_base::binary | std::ios_base::out);
    if(!bank.is_open())
    {
        //std::cerr << std::strerror(errno) << std::endl;
        return ErrorNum::FailedToWriteFile;
    }
    //bank.write(reinterpret_cast<char*>(&bufferSize), sizeof(uint32_t));

    char* buffer = new char[bufferSize];

    uint64_t offset = 0;

    // Store all data in bank
    for(auto& file : filesToEncode)
    {
        if(file.encoding == Encoding::PCM)
        {
            offset += WritePCM(buffer + offset, file);
        }
        else if(file.encoding == Encoding::Opus)
        {
            offset += WriteOpus(buffer + offset, file);
        }
    }

    bank.write(buffer, offset);
    bank.close();
    delete [] buffer;

    return ErrorNum::NoErrors;
}

int PackageEncoder::WritePCM(char* buffer, FileInfo& file)
{
    *reinterpret_cast<uint64_t *>(buffer) = file.id;
    int offset = sizeof (uint64_t);

    WavFile wavFile(file.wavPath);

    // Write riff header
    RiffHeader riffHeader{{'R','I','F','F'},
                          0,
                          {'W','A','V','E'}};
    riffHeader.riff_size = sizeof(WavHeader) + wavFile.GetDataSize();
    *reinterpret_cast<RiffHeader*>(buffer + offset) = riffHeader;
    offset += sizeof(RiffHeader);

    // Write fmt header
    *reinterpret_cast<FormatHeader*>(buffer + offset) = wavFile.GetFormat();
    (*reinterpret_cast<FormatHeader*>(buffer + offset)).bits_per_sample = sizeof(float) * 8;
    offset += sizeof(FormatHeader);

    // TODO any proccesing like resmapleing would be done before this step

    // Write data header
    GenericHeaderChunk dataChunk{{'d', 'a','t','a'}, 0};
    dataChunk.chunkSize = (wavFile.GetDataSize() / (wavFile.GetFormat().bits_per_sample / 8)) * sizeof(float);
    *reinterpret_cast<GenericHeaderChunk*>(buffer + offset) = dataChunk;
    offset += sizeof (GenericHeaderChunk);

    // Write data
    wavFile.GetDataAsFloat(reinterpret_cast<float*>(buffer + offset));
    offset += dataChunk.chunkSize;
    return offset;
}

int PackageEncoder::WriteOpus(char* buffer, FileInfo& file)
{
    *reinterpret_cast<uint64_t *>(buffer) = file.id;
    int offset = sizeof (uint64_t);

    WavFile wavFile(file.wavPath);

    FormatHeader fmtHeader = wavFile.GetFormat();
    OpusHeaderChunk headerChunk {{'O', 'p', 'u', 's', 'H', 'e', 'a', 'd'}};
    headerChunk.channels = fmtHeader.channel_count;
    headerChunk.sampleRate = fmtHeader.sampling_rate;
    headerChunk.mapingFamily = 0;
    headerChunk.outputGain = 0;
    headerChunk.preSkip = 0;
    headerChunk.version = 1;

    *reinterpret_cast<OpusHeaderChunk*>(buffer + offset) = headerChunk;
    offset += 19;
    // TODO test if in memory buffer works so no need for secound buffer

    offset += wavFile.GetDataAsOpus(buffer + offset);

    return offset;
}