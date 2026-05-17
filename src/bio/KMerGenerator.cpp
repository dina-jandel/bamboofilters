// this code was written by team member: Marija Špoljarić
#include "KMerGenerator.hpp"

// (k-mer: a substring of a DNA sequence of length k)
/* e.g.:
    DNA sequence: ACGTAG
    k = 3

    k-mers:
    ACG
    CGT
    GTA
    TAG
*/

// function that generates all possible k-mers for a given DNA sequence
std::vector<std::string> generateKmers(const std::string &dna, int k)
{
    // vector of strings that will store all generated k-mers
    std::vector<std::string> kmers;

    for (size_t i = 0; i + k <= dna.size(); i++)
    {
        // takes a substring starting at index i with length k
        // adds that k-mer to the vector
        kmers.push_back(dna.substr(i, k));
    }

    return kmers;
}