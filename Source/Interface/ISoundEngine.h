//
// Created by zack on 10/25/21.
//

#ifndef I_SOUND_ENGINE_ISOUNDENGINE_H
#define I_SOUND_ENGINE_ISOUNDENGINE_H

#include "AudioPackage/PackageManager.h"
#include "Events/EventManager.h"
#include "RealTimeAudio/RealTimeAudio.h"

class ISoundEngine
{
public:
    /*!
     * Contruct the sound engine
     */
    ISoundEngine();

    /*!
     * Initialize all systems to be ready to run real time audio
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ErrorNum Init();
    /*!
     * Handles and updates all needed buffers like an event buffer.
     * @return
     */
    ErrorNum Update();
    /*!
     * Shuts engine down and stops real time audio
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ErrorNum Shutdown();

    /*!
     * Loads a package of audio data for use with events
     * @param path Path to the package to load
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ErrorNum LoadPackage(std::string path);
    /*!
     * Unloads a package and make it no longer usable with events
     * @param path Path of the bank to unload
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ErrorNum UnloadPackage(std::string path);

    /*!
     * Load events structs for later posting
     * @param path Path to events
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ErrorNum LoadEvents(std::string path);
    /*!
     * Remove events from currently available events
     * @param path Path of events to unload
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ErrorNum UnloadEvents(std::string path);

    /*!
     * Puts event into buffer to play, starts playing when update is called
     * @param EventName Name of the event
     * @return ID of the created event
     */
    uint64_t PostEvent(std::string EventName);
    /*!
    * Puts event into buffer to play, starts playing when update is called
    * @param EventName ID of the event
    * @return ID of the created event
    */
    uint64_t PostEvent(uint64_t id);


private:
    PackageManager packageManager;
    EventManager eventManager;
    RealTimeAudio realTimeAudio;
};


#endif //I_SOUND_ENGINE_ISOUNDENGINE_H
