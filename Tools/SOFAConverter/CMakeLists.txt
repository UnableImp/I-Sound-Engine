
add_executable(SOFAConverter SOFAConverter.cpp)

target_link_libraries(SOFAConverter PUBLIC SOFA ISoundEngine mysofa-static)
target_include_directories(SOFAConverter PUBLIC SOFA mysofa-static)

# copy needed dlls from sofa to exe
add_custom_command( TARGET SOFAConverter POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/Libraries/libsofa/libsofa/dependencies/lib/win/x64 ${CMAKE_CURRENT_BINARY_DIR})