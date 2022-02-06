set(ISE_SOURCES
  "AudioFormats/OpusDecoderWrapper.h"
  "AudioFormats/OpusDecoderWrapper.cpp"
  "AudioFormats/OpusEncoderWrapper.h"
  "AudioFormats/OpusEncoderWrapper.cpp"
  "AudioFormats/OpusFile.h"
  "AudioFormats/OpusFile.cpp"
  "AudioFormats/OpusHeader.h"
  "AudioFormats/WavFile.h"
  "AudioFormats/WavFile.cpp"
  "AudioFormats/WavHeader.h"
  "AudioPackage/PackageDecoder.h"
  "AudioPackage/PackageDecoder.cpp"
  "AudioPackage/PackageEncoder.h"
  "AudioPackage/PackageEncoder.cpp"
  "Filters/OpusContainer.h"
  "Filters/SoundContainer.h"
  "Filters/WavContainer.h"
  "IO/IOUtility.h"
  "IO/MemoryMappedFile.h"
  "ErrorList.h"
  "SoundData.h"
  "Events/Event.h"
  "Events/Event.cpp"
  "AudioFrame.h" AudioPackage/PackageManager.cpp AudioPackage/PackageManager.h Events/EventManager.cpp Events/EventManager.h RealTimeAudio/RealTimeAudio.cpp RealTimeAudio/RealTimeAudio.h Events/EventParser.cpp Events/EventParser.h Events/FilterParsers/DeserializeFilter.h Interface/ISoundEngine.cpp Interface/ISoundEngine.h Filters/ConvolutionFreq.h Filters/HRIRCalculator.h RealTimeParameters/Transform.h RealTimeParameters/IVector3.h RealTimeParameters/GameObjectManager.cpp RealTimeParameters/GameObjectManager.h RealTimeParameters/GameObject.cpp RealTimeParameters/GameObject.h
        Events/FilterParsers/Deserialzied3DSound.cpp Events/FilterParsers/Deserialized3DSound.h Events/FilterParsers/DeserializedSound.cpp Actions/Action.h Actions/PostEvent.h Actions/StopEvent.h Filters/ITD.h Events/FilterParsers/DeserializedITD.cpp Events/FilterParsers/DeserializedITD.h)

# append platform specific sources
if (OS_Windows)
  set(ISE_SOURCES ${ISE_SOURCES}
    "IO/Win32/IOUtilityWin32.cpp"
    "IO/Win32/MemoryMappedFileWin32.cpp"
  )
elseif(OS_Linux)
  set(ISE_SOURCES ${ISE_SOURCES}
    "IO/Linux/IOUtilityLinux.cpp"
    "IO/Linux/MemoryMappedFileLinux.cpp"
          )
endif()

add_library(ISoundEngine STATIC ${ISE_SOURCES})
add_library(ISoundEngine_dynamic ${ISE_SOURCES})

target_include_directories(ISoundEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ISoundEngine opus PortAudio rapidjson PFFFT)

target_include_directories(ISoundEngine_dynamic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ISoundEngine_dynamic opus PortAudio rapidjson PFFFT)

add_compile_definitions(ISoundEngine_dynamic "ISE_EXPORTS")
