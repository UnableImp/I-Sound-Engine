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
)

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

add_library(ISoundEngine ${ISE_SOURCES})
target_include_directories(ISoundEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ISoundEngine opus)
