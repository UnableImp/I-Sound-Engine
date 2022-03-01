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

add_library(PFFFT ${FLOAT_SOURCES} pffft/pffft_common.c pffft/pffft_priv_impl.h pffft/pffft.hpp)

set_target_properties(PFFFT PROPERTIES OUTPUT_NAME "pffft")
target_compile_definitions(PFFFT PRIVATE _USE_MATH_DEFINES)
target_compile_definitions(PFFFT PRIVATE PFFFT_SCALVEC_ENABLED=1)

#if ( CMAKE_C_COMPILER_ID MATCHES "MSVC" )
#    set(MATHLIB "")
#else()
#    set(MATHLIB "m")
#endif()

target_link_libraries( PFFFT ${MATHLIB} )
target_include_directories(PFFFT PUBLIC pffft)

#rapidjson
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE rapidjson/include)

#sofa
add_library(SOFA INTERFACE)
target_include_directories(SOFA INTERFACE libsofa/libsofa/dependencies/include)
target_include_directories(SOFA INTERFACE libsofa/libsofa/src)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    target_link_libraries(SOFA INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/libsofa/libsofa/dependencies/lib/win/libcurl_imp_x64.lib
            ${CMAKE_CURRENT_SOURCE_DIR}/libsofa/libsofa/dependencies/lib/win/libhdf5_hl_x64.lib
            ${CMAKE_CURRENT_SOURCE_DIR}/libsofa/libsofa/dependencies/lib/win/netcdf_x64.lib
            ${CMAKE_CURRENT_SOURCE_DIR}/libsofa/libsofa/dependencies/lib/win/zlib_x64.lib)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(SOFA INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libsofa/libsofa/lib/libsofa_debug_x64.lib)
    else()
        target_link_libraries(SOFA INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libsofa/libsofa/lib/libsofa_x64.lib)
    endif()

endif()

add_subdirectory(libmysofa)
target_include_directories(mysofa-static PUBLIC libmysofa/src/hrtf)