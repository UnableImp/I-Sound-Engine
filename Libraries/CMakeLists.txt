add_subdirectory(Opus)

add_subdirectory(googletest)

option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." ON)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library." FORCE)
add_subdirectory(benchmark)
