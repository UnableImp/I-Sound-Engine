//
// Created by zack on 9/23/21.
//

#ifndef I_SOUND_ENGINE_PACKAGEDECODER_H
#define I_SOUND_ENGINE_PACKAGEDECODER_H

#include "ErrorList.h"
#include "SoundData.h"
#include <unordered_map>

namespace PackageDecoder
{
    /*!
     * Will decode a given package and map the table to have sound ids pointing to there respect sound
     * @param lookUpTable The table to fill with info
     * @param data Decoder will create and fill its own buffer and store here, you are responsible for freeing the memory
     * @param path Filepath to the bank to decode
     * @return NoErrors - there were no errors
     *         Anything else there was errors with decoding
     */
    ErrorNum DecodePackage(std::unordered_map<uint64_t, SoundData>& lookUpTable, char** data, std::string path);

    ErrorNum ReleasePackage(std::unordered_map<uint64_t, SoundData>& lookUpTable, char** data);

};

#endif //I_SOUND_ENGINE_PACKAGEDECODER_H
