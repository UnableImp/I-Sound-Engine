//
// Created by zack on 10/19/21.
//

#include "PackageManager.h"
#include "IO/IOUtility.h"
#include "PackageDecoder.h"

ErrorNum PackageManager::LoadPack(std::string path)
{
    if(IO::FileExists(path))
    {
        packages[path] = new IO::MemoryMappedFile(path);
        ErrorNum error = PackageDecoder::DecodePackage(sounds, *packages[path]);
        if(error == ErrorNum::NoErrors)
            return ErrorNum::NoErrors;
        delete packages[path];
        packages.erase(path);
        return error;
    }
    return ErrorNum::FailedToFindFile;
}

ErrorNum PackageManager::UnloadPack(std::string path)
{
    auto iter = packages.find(path);
    if(iter == packages.end())
        return ErrorNum::PackageNotLoaded;

    //PackageDecoder::ReleasePackage(sounds, )
    delete iter->second;
    packages.erase(iter);
    return ErrorNum::NoErrors;

}

std::unordered_map<uint64_t, SoundData> &PackageManager::GetSounds()
{
    return sounds;
}

PackageManager::~PackageManager()
{
    for(auto& iter : packages)
    {
        delete iter.second;
    }
}