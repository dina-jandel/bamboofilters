// this code was written by team member: Marija Špoljarić
#include "FASTAParser.hpp"
#include <fstream>
#include <iostream>
#include <cctype>

// function that reads a fasta file and extracts dna sequence
std::string readFasta(const std::string &filename)
{
    // open input file stream
    std::ifstream file(filename);

    // check if file was successfully opened
    if (!file.is_open())
    {
        std::cerr << "error: cannot open fasta file: " << filename << "\n";
        return "";
    }

    std::string line;
    std::string dna;

    // reserve approximate memory to reduce reallocations
    dna.reserve(1000000);

    // read file line by line
    while (std::getline(file, line))
    {
        // skip header lines and empty lines
        if (line.empty() || line[0] == '>')
            continue;

        // process each character in the line
        for (char c : line)
        {
            // convert to uppercase
            c = std::toupper(c);

            // keep only valid dna nucleotides
            if (c == 'A' || c == 'C' || c == 'G' || c == 'T')
                dna.push_back(c);

            // ignore all other characters (e.g. N and ambiguous bases)
        }
    }

    return dna;
}