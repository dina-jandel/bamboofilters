#include "Benchmark.hpp"

void Benchmark::start()
{
    startTime = std::chrono::high_resolution_clock::now();
}

long long Benchmark::stop()
{
    auto endTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(
               endTime - startTime)
        .count();
}
