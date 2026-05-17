// this code was written by team member: Marija Špoljarić
#include "DNAGenerator.hpp" // header with function declarations
#include <random>
#include <string>
#include <cstdlib> // for the rand() function

std::string generateRandomDNA(int k)
{                                                      // function generates a random DNA sequence of length k
    static const char bases[4] = {'A', 'C', 'G', 'T'}; // bases: adenine, cytosine, guanine, thymine

    std::string result; // string for storing a generated DNA sequence
    result.reserve(k);  // preallocates memory for k characters, improves performance

    for (int i = 0; i < k; i++)
    {                                        // generates (randomly) one of the bases k times and stores it in the result string
        result.push_back(bases[rand() % 4]); // rand()%4 returns a number 0, 1, 2, or 3 (index of the array where the bases are stored)
    }

    return result; // pov.vr. -> slucajno generirana DNA sekvenca
}

std::string generateDNA(size_t n)
{                                               // function generates a random DNA sequence of length n
    const char bases[4] = {'A', 'C', 'G', 'T'}; // bases
    std::string result;
    result.reserve(n); // preallocation for n characters

    // random_device generates a seed for the random number generator (the seed is used to produce different results each time the program runs)
    std::random_device rd;
    // Mersenne Twister random number generator
    std::mt19937 gen(rd());
    // generator provides a uniform selection of numbers between 0 and 3 (each base has an equal probability of occurrence)
    std::uniform_int_distribution<> dist(0, 3);

    for (size_t i = 0; i < n; i++)
    {
        result += bases[dist(gen)]; // dist(gen) -> generates a number 0, 1, 2, or 3
    }

    return result; // return value -> randomly generated DNA sequence
}