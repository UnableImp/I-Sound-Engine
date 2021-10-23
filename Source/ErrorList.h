//
// Created by zacke on 8/31/2021.
//

#ifndef I_SOUND_ENGINE_ERRORLIST_H
#define I_SOUND_ENGINE_ERRORLIST_H

enum ErrorNum
{
    NoErrors,
    FailedToFindFile,
    NotAPCMFile,
    FailedToWriteFile,
    UnsportedFile,
    OpusError,
    EventNotFound,
    SoundNotLoaded
};

#endif //I_SOUND_ENGINE_ERRORLIST_H
