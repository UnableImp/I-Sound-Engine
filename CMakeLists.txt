cmake_minimum_required(VERSION 3.20)
project(I_Sound_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# define the macro DEBUG if compiling in that configuration
add_compile_definitions("$<$<CONFIG:DEBUG>:DEBUG>")

if (NOT (OS_Windows OR OS_Linux OR OS_Horizon))
# define OS macros
if (WIN32)
    add_compile_definitions("$<${WIN32}:OS_Windows>")
    set(OS_Windows TRUE)
elseif(UNIX)
    add_compile_definitions("$<$<PLATFORM_ID:Linux>:OS_Linux>")
    set(OS_Linux $<PLATFORM_ID:Linux>)
endif ()

endif()

add_subdirectory(Libraries)
enable_testing()
add_subdirectory(Source)
add_subdirectory(Testing)
