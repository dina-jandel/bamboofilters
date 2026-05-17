// this code was written by team member: marija špoljarić
#include "Benchmark.hpp"

#include <chrono>

// benchmark utility for measuring execution time
// used to measure:
// - insert performance
// - lookup performance
// - runtime comparison between filters

// start timing
void Benchmark::start()
{
    // records the starting time point using high resolution clock
    startTime = std::chrono::high_resolution_clock::now();
}

// stop timing
long long Benchmark::stop()
{
    // records the ending time point
    auto endTime = std::chrono::high_resolution_clock::now();

    // computes elapsed time in milliseconds
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               endTime - startTime)
        .count();
}