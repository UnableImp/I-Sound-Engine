//
// Created by zack on 9/29/21.
//

#include "OpusFile.h"

#include <iostream>
#include <cstring>

OpusFile::OpusFile(const std::string &path) : errorState(ErrorNum::NoErrors),
                                              opusFile(path, std::ios_base::binary | std::ios_base::in)
{
    if(!opusFile)
    {
        errorState = ErrorNum::FailedToFindFile;
        return;
    }

    opusFile.read(reinterpret_cast<char*>(&opusHeader), sizeof(opusHeader) - 1);
    // TODO Read memory mapping

    opusFile.read(commentHeader.signature, sizeof(commentHeader.signature));
    opusFile.read(reinterpret_cast<char*>(&commentHeader.vendorStringLength), sizeof(unsigned ));
    if(commentHeader.vendorStringLength > 0)
    {
        commentHeader.vendorString = new char[commentHeader.vendorStringLength];
        opusFile.read(commentHeader.vendorString, commentHeader.vendorStringLength);
    }

    opusFile.read(reinterpret_cast<char*>(&commentHeader.numOfUsercomments), sizeof(commentHeader.numOfUsercomments));
    if(commentHeader.numOfUsercomments > 0)
    {
        commentHeader.userCommentsLength = new unsigned[commentHeader.numOfUsercomments];
        commentHeader.userComments = new char*[commentHeader.numOfUsercomments];
        for(int i = 0; i < commentHeader.numOfUsercomments; ++i)
        {
            opusFile.read(reinterpret_cast<char*>(commentHeader.userCommentsLength + i), sizeof(unsigned));
            commentHeader.userComments[i] = new char[commentHeader.userCommentsLength[i]];
            opusFile.read(commentHeader.userComments[i], commentHeader.userCommentsLength[i]);
        }
    }

    //TODO any work depending on user comments before clean up

    for(int i = 0; i < commentHeader.numOfUsercomments; ++i)
    {
        std::cout << commentHeader.userComments[i] << std::endl;
    }
}

OpusFile::~OpusFile()
{
    if(commentHeader.vendorStringLength > 0)
        delete [] commentHeader.vendorString;


    for(int i = 0; i < commentHeader.numOfUsercomments; ++i)
    {
        delete [] commentHeader.userComments[i];
    }

    if(commentHeader.numOfUsercomments > 0)
    {
        delete [] commentHeader.userComments;
        delete [] commentHeader.userCommentsLength;
    }
}

void OpusFile::WriteToFile(char* opusData, int size, OpusHeaderChunk header, char* fileName)
{
    OggS OggSHeader{'O', 'g', 'g', 'S'};
    OggSHeader.version = 0;
    memset(OggSHeader.gPosition, 0, 8);
    OggSHeader.pageSegments = 1;
    OggSHeader.seirlNumber = 0;
    OggSHeader.CheckSum = 0;
    OggSHeader.headerTypes = 0x2;
    OggSHeader.segmationTable = 19; // Magic OpusHeader Size;

    std::fstream file(fileName, std::ios_base::binary | std::ios_base::out);

    int sizeOggs = sizeof(OggS);
    file.write(OggSHeader.signature, sizeof(OggSHeader.signature));
    file.write(&OggSHeader.version, sizeof(OggSHeader.version));
    file.write(&OggSHeader.headerTypes, sizeof(OggSHeader.headerTypes));
    file.write(OggSHeader.gPosition, sizeof(OggSHeader.gPosition));
    file.write(reinterpret_cast<char*>(&OggSHeader.seirlNumber), sizeof(OggSHeader.seirlNumber));
    file.write(reinterpret_cast<char*>(&OggSHeader.pageSequenceNumber), sizeof(OggSHeader.pageSequenceNumber));
    file.write(reinterpret_cast<char*>(&OggSHeader.CheckSum), sizeof(OggSHeader.CheckSum));
    file.write(&OggSHeader.pageSegments, sizeof(OggSHeader.pageSegments));
    file.write(&OggSHeader.segmationTable, sizeof(OggSHeader.segmationTable));
    file.write(reinterpret_cast<char *>(&header), 19);

    CommentHeader cHeader{'O','p','u','s','T','a','g','s'};
    char MyVender[] = {"Imp"};
    cHeader.vendorStringLength = 3;
    cHeader.numOfUsercomments = 0;

    OggSHeader.headerTypes = 0;
    OggSHeader.pageSequenceNumber = 1;
    OggSHeader.segmationTable = 19;

    file.write(OggSHeader.signature, sizeof(OggSHeader.signature));
    file.write(&OggSHeader.version, sizeof(OggSHeader.version));
    file.write(&OggSHeader.headerTypes, sizeof(OggSHeader.headerTypes));
    file.write(OggSHeader.gPosition, sizeof(OggSHeader.gPosition));
    file.write(reinterpret_cast<char*>(&OggSHeader.seirlNumber), sizeof(OggSHeader.seirlNumber));
    file.write(reinterpret_cast<char*>(&OggSHeader.pageSequenceNumber), sizeof(OggSHeader.pageSequenceNumber));
    file.write(reinterpret_cast<char*>(&OggSHeader.CheckSum), sizeof(OggSHeader.CheckSum));
    file.write(&OggSHeader.pageSegments, sizeof(OggSHeader.pageSegments));
    file.write(&OggSHeader.segmationTable, sizeof(OggSHeader.segmationTable));

    file.write(cHeader.signature,8);
    file.write(reinterpret_cast<char*>(&cHeader.vendorStringLength), sizeof(cHeader.vendorStringLength));
    file.write(MyVender, 3);
    file.write(reinterpret_cast<char*>(&cHeader.numOfUsercomments), sizeof(cHeader.numOfUsercomments));


    OggSHeader.pageSequenceNumber = 2;
    *reinterpret_cast<short*>(&OggSHeader.gPosition[0]) = 960;
    OggSHeader.headerTypes = 0x4;

    file.write(OggSHeader.signature, sizeof(OggSHeader.signature));
    file.write(&OggSHeader.version, sizeof(OggSHeader.version));
    file.write(&OggSHeader.headerTypes, sizeof(OggSHeader.headerTypes));
    file.write(OggSHeader.gPosition, sizeof(OggSHeader.gPosition));
    file.write(reinterpret_cast<char*>(&OggSHeader.seirlNumber), sizeof(OggSHeader.seirlNumber));
    file.write(reinterpret_cast<char*>(&OggSHeader.pageSequenceNumber), sizeof(OggSHeader.pageSequenceNumber));
    file.write(reinterpret_cast<char*>(&OggSHeader.CheckSum), sizeof(OggSHeader.CheckSum));
    file.write(&OggSHeader.pageSegments, sizeof(OggSHeader.pageSegments));

    int value = *reinterpret_cast<int*>(opusData);
    unsigned char valueToPrint = 255;

    while(value > 0 )
    {
        if(value < 255)
            valueToPrint = value;
        file.write(reinterpret_cast<char*>(&valueToPrint), sizeof(char));
        value -= valueToPrint;
    }

    file.write(opusData + sizeof(uint32_t), *reinterpret_cast<uint32_t*>(opusData));

}

int OpusFile::WriteOggStoBuffer(char* buffer, OggS header)
{
    memcpy(buffer, header.signature, 4);
    int offset = sizeof(OggS::signature);
    *(buffer+offset) = header.version;
    offset += sizeof(OggS::version);
    *(buffer+offset) = header.headerTypes;
    memcpy(buffer + offset, header.gPosition, sizeof(OggS::gPosition));
    offset += sizeof(OggS::gPosition);
    *reinterpret_cast<int*>(buffer+offset) = header.seirlNumber;
    offset += sizeof(int);
    *reinterpret_cast<int*>(buffer+offset) = header.pageSequenceNumber;
    offset += sizeof(int);
    *reinterpret_cast<int*>(buffer+offset) = header.CheckSum;
    offset += sizeof(int);
    *(buffer+offset) = header.pageSegments;
    offset += sizeof(char);

    return offset;
}

int OpusFile::GetSegementSize(char *buffer, int& packetSize)
{
    int tableIndex = 26; // Magic number to segmentation table
    int opusPacketSize = *reinterpret_cast<unsigned char*>((buffer + tableIndex));
    ++tableIndex;
    int readSize = opusPacketSize;

    // size == 255 means that there are more bytes need to create this segment
    while(readSize == 255)
    {
        readSize = *reinterpret_cast<unsigned char*>((buffer + tableIndex));
        opusPacketSize += readSize;
        ++tableIndex;
    }
    packetSize = opusPacketSize;
    return tableIndex;
}
