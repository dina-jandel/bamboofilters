#pragma once

#include <chrono>

class Benchmark
{
public:
    void start();
    long long stop(); // vraća vrijeme u ms

private:
    std::chrono::high_resolution_clock::time_point startTime;
};