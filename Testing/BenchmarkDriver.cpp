//
// Created by zack on 9/28/21.
//

#include "benchmark/benchmark.h"
//--benchmark_filter=
int main(int argc, char** argv)
{
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
}