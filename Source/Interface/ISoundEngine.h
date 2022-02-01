//
// Created by zack on 10/25/21.
//

#ifndef I_SOUND_ENGINE_ISOUNDENGINE_H
#define I_SOUND_ENGINE_ISOUNDENGINE_H

#ifdef OS_Windows
#ifdef ISE_EXPORTS
#define ISE_API __declspec(dllexport)
#else
#define ISE_API __declspec(dllimport)
#endif
#else
#define ISE_API
#endif

#include "AudioPackage/PackageManager.h"
#include "Events/EventManager.h"
#include "RealTimeAudio/RealTimeAudio.h"
#include "RealTimeParameters/GameObjectManager.h"

extern "C"
{
namespace ISoundEngine
{

    ISE_API ErrorNum StartUp();

    /*!
     * Initialize all systems to be ready to run real time audio
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ISE_API ErrorNum Init();
    /*!
     * Handles and updates all needed buffers like an event buffer.
     * @return
     */
    ISE_API ErrorNum Update();
    /*!
     * Shuts engine down and stops real time audio
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ISE_API ErrorNum Shutdown();

    /*!
     * Loads a package of audio data for use with events
     * @param path Path to the package to load
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ISE_API ErrorNum LoadPackage(char* path);
    /*!
     * Unloads a package and make it no longer usable with events
     * @param path Path of the bank to unload
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ISE_API ErrorNum UnloadPackage(char* path);

    /*!
     * Load events structs for later posting
     * @param path Path to events
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ISE_API ErrorNum LoadEvents(char* path);
    /*!
     * Remove events from currently available events
     * @param path Path of events to unload
     * @return NoErrors - on success, else failed, check error code for resound
     */
    ISE_API ErrorNum UnloadEvents(char* path);

    /*!
     * Puts event into buffer to play, starts playing when update is called
     * @param EventName Name of the event
     * @return ID of the created event
     */
    ISE_API uint64_t PostEventString(char* EventName);
    /*!
    * Puts event into buffer to play, starts playing when update is called
    * @param EventName ID of the event
    * @return ID of the created event
    */
    ISE_API uint64_t PostEvent(uint64_t id);

    ISE_API uint64_t PostEventObject(uint64_t id, uint64_t gameobject);

    /*!
     * Adds a game object to internals, required for 3d sounds
     * @param id ID to reference the obj by
     * @return 0 if failed to insert, else id
     */
    ISE_API uint64_t AddObject(uint64_t id);

    /*!
     * Removes a game object when no longer in use
     * @param id ID of object to remove
     * @return 0 if removed failed, else id
     */
    ISE_API uint64_t RemoveObject(uint64_t id);

    /*!
     * Sets transform of a game object
     * @param id ID of object's transform to update
     * @param transform Transform to set it too
     */
    ISE_API void SetTransform(uint64_t id, const Transform &transform);

    /*!
     * Sets just the postion part of a game object
     * @param id ID of object to update
     * @param position Postion to update too
     */
    ISE_API void SetPosition(uint64_t id, const IVector3 &position);

    ISE_API void SetListenerTransform(const Transform &transform);

    ISE_API void SetListernerPosition(const IVector3 &position);

    ISE_API void SetParam(const char* id, float value);

    ISE_API float GetParam(const char* id);
    }
};


#endif //I_SOUND_ENGINE_ISOUNDENGINE_H
