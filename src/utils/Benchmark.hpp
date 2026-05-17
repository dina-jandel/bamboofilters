#pragma once
// this code was written by team member: marija špoljarić

#include <chrono>

// benchmark class used for measuring execution time
// used to evaluate performance of data structures:
// - insertion time
// - lookup time
// - general runtime comparison

class Benchmark
{
public:
    // starts the timer
    void start();

    // stops the timer and returns elapsed time in milliseconds
    long long stop();

private:
    // starting time point for benchmark measurement
    std::chrono::high_resolution_clock::time_point startTime;
};