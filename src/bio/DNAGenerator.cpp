#include "DNAGenerator.hpp"
#include <random>

#include <string>
#include <cstdlib>

std::string generateRandomDNA(int k) {
    static const char bases[4] = {'A', 'C', 'G', 'T'};

    std::string result;
    result.reserve(k);

    for (int i = 0; i < k; i++) {
        result.push_back(bases[rand() % 4]);
    }

    return result;
}


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
