if (OS_Windows)
    set(BUILD_SHARED_LIBS OFF)
endif()

add_subdirectory(portaudio)
add_subdirectory(Opus)
add_subdirectory(googletest)

option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." ON)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library." FORCE)
add_subdirectory(benchmark)

#Pffft - Use SIMD and floats
set( SIMD_FLOAT_HDRS pffft/simd/pf_float.h pffft/simd/pf_sse1_float.h pffft/simd/pf_altivec_float.h pffft/simd/pf_neon_float.h pffft/simd/pf_scalar_float.h )
set( FLOAT_SOURCES pffft/pffft.c pffft/pffft.h ${SIMD_FLOAT_HDRS} )

add_library(PFFFT STATIC ${FLOAT_SOURCES} pffft/pffft_common.c pffft/pffft_priv_impl.h pffft/pffft.hpp)

set_target_properties(PFFFT PROPERTIES OUTPUT_NAME "pffft")
target_compile_definitions(PFFFT PRIVATE _USE_MATH_DEFINES)
target_compile_definitions(PFFFT PRIVATE PFFFT_SCALVEC_ENABLED=1)
set(MATHLIB "m")
target_link_libraries( PFFFT ${MATHLIB} )
target_include_directories(PFFFT PUBLIC pffft)


add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE rapidjson/include)