#include "KMerGenerator.hpp"

std::vector<std::string> generateKmers(const std::string &dna, int k)
{
    std::vector<std::string> kmers;

    for (size_t i = 0; i + k <= dna.size(); i++)
    {
        kmers.push_back(dna.substr(i, k));
    }

    return kmers;
}