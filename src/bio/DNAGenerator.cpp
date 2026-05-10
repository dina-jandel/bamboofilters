#include "DNAGenerator.hpp"
#include <random>

std::string generateDNA(size_t n)
{
    const char bases[4] = {'A', 'C', 'G', 'T'};
    std::string result;
    result.reserve(n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 3);

    for (size_t i = 0; i < n; i++)
    {
        result += bases[dist(gen)];
    }

    return result;
}