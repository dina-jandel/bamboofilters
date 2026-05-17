// this code was written by team member: Marija Špoljarić
#ifndef DNA_GENERATOR_HPP
#define DNA_GENERATOR_HPP

#include <string>

// declaration of a function that generates a DNA sequence of a given length using a modern random generator
std::string generateDNA(size_t length);

// declaration of a function that generates a DNA sequence using the classic rand()
std::string generateRandomDNA(int k);

#endif