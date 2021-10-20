add_executable(ISETests "TestDriver.cpp" "Modules/ContainerModule.cpp" "Modules/PackageModule.cpp" Modules/EventModule.cpp Modules/EventModule.h)
add_executable(ISEBenchmark "BenchmarkDriver.cpp" "Modules/ContainerModule.cpp" "Modules/PackageModule.cpp" Modules/EventModule.cpp Modules/EventModule.h)

target_link_libraries(ISETests gtest benchmark::benchmark ISoundEngine)

target_link_libraries(ISEBenchmark gtest benchmark::benchmark ISoundEngine)
