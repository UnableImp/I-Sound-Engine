set(BUILD_SHARED_LIBS OFF)
add_subdirectory(portaudio)
add_subdirectory(Opus)
add_subdirectory(googletest)

option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." ON)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library." FORCE)
add_subdirectory(benchmark)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE rapidjson/include)