
add_executable(SOFAConverter SOFAConverter.cpp)

target_link_libraries(SOFAConverter PUBLIC ISoundEngine mysofa-static)
target_include_directories(SOFAConverter PUBLIC mysofa-static)

# copy needed dlls from sofa to exe
#add_custom_command( TARGET SOFAConverter POST_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_directory
#                    ${CMAKE_SOURCE_DIR}/Libraries/libmysofa/windows/third-party/zlib-1.2.11/bin/ ${CMAKE_CURRENT_BINARY_DIR})