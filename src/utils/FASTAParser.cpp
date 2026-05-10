#include "FASTAParser.hpp"
#include <fstream>
#include <iostream>
#include <cctype>

std::string readFasta(const std::string &filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "ERROR: Cannot open FASTA file: " << filename << "\n";
        return "";
    }

    std::string line;
    std::string dna;
    dna.reserve(1000000);

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '>')
            continue;

        for (char c : line)
        {
            c = std::toupper(c);

            if (c == 'A' || c == 'C' || c == 'G' || c == 'T')
                dna.push_back(c);
            // N i ostalo ignoriramo eksplicitno
        }
    }

    return dna;
}