//
// Created by zack on 10/19/21.
//

#ifndef I_SOUND_ENGINE_PACKAGEMANAGER_H
#define I_SOUND_ENGINE_PACKAGEMANAGER_H


#include <unordered_map>
#include <string>
#include "IO/MemoryMappedFile.h"
#include "SoundData.h"
#include "ErrorList.h"

class PackageManager
{
public:
    ErrorNum LoadPack(std::string path);
    ErrorNum UnloadPack(std::string path);

    std::unordered_map<uint64_t, SoundData>& GetSounds();

    ~PackageManager();

private:
    std::unordered_map<std::string, IO::MemoryMappedFile*> packages;
    std::unordered_map<uint64_t, SoundData> sounds;
};


#endif //I_SOUND_ENGINE_PACKAGEMANAGER_H
