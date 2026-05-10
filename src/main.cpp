#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <random>
#include <algorithm>

#include "filter/BambooFilter.hpp"
#include "bio/DNAGenerator.hpp"
#include "bio/KMerGenerator.hpp"
#include "utils/Benchmark.hpp"
#include "utils/FASTAParser.hpp"

// generates random dna sequence used for negative testing
std::string generateRandomDNA(int k)
{
    static const char bases[4] = {'A', 'C', 'G', 'T'};

    std::string result;
    result.reserve(k);

    for (int i = 0; i < k; i++)
    {
        result.push_back(bases[rand() % 4]);
    }

    return result;
}

int main(int argc, char *argv[])
{
    srand(42); // fixed seed for reproducibility

    // default configuration values for experiment
    std::string inputFile = "";
    int k = 20;
    size_t filterSize = 1000000;
    int hashCount = 5;
    std::string outputFile = "results.txt";
    size_t syntheticDNASize = 100000;

    // parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--file")
        {
            inputFile = argv[++i];
        }
        else if (arg == "--k")
        {
            k = std::stoi(argv[++i]);
        }
        else if (arg == "--size")
        {
            filterSize = std::stoul(argv[++i]);
        }
        else if (arg == "--hashes")
        {
            hashCount = std::stoi(argv[++i]);
        }
        else if (arg == "--out")
        {
            outputFile = argv[++i];
        }
    }

    // initialize bloom/bamboo filter structure
    BambooFilter filter(filterSize, hashCount);

    // load input dna sequence either from file or synthetic generator
    std::string dnaSequence;

    if (inputFile.empty())
    {
        dnaSequence = generateDNA(syntheticDNASize);
        std::cout << "using synthetic dna\n";
    }
    else
    {
        dnaSequence = readFasta(inputFile);
        std::cout << "using fasta file: " << inputFile << "\n";
    }

    // check if dna was successfully loaded
    if (dnaSequence.empty())
    {
        std::cerr << "error: dna sequence is empty\n";
        return 1;
    }

    std::cout << "dna size: " << dnaSequence.size() << "\n";

    // generate k-mers from dna sequence
    auto kmers = generateKmers(dnaSequence, k);
    std::cout << "k-mers: " << kmers.size() << "\n";

    // benchmark insertion phase into filter
    Benchmark insertBenchmark;
    insertBenchmark.start();

    for (const auto &kmer : kmers)
    {
        filter.insert(kmer);
    }

    long long insertTime = insertBenchmark.stop();

    // benchmark lookup phase using same k-mers (true positives)
    Benchmark lookupBenchmark;
    lookupBenchmark.start();

    int truePositives = 0;

    for (const auto &kmer : kmers)
    {
        if (filter.contains(kmer))
        {
            truePositives++;
        }
    }

    long long lookupTime = lookupBenchmark.stop();

    // evaluate false positives using random dna sequences
    int falsePositives = 0;
    int testSamples = 100000;

    for (int i = 0; i < testSamples; i++)
    {
        std::string fakeKmer = generateRandomDNA(k);

        if (filter.contains(fakeKmer))
        {
            falsePositives++;
        }
    }

    double falsePositiveRate = (double)falsePositives / testSamples * 100.0;

    // compute load factor of filter
    double loadFactor = (double)kmers.size() / filterSize;

    // write results to output file
    std::ofstream out(outputFile);

    out << "insert time [ms]: " << insertTime << "\n";
    out << "lookup time [ms]: " << lookupTime << "\n";
    out << "dna size: " << dnaSequence.size() << "\n";
    out << "k-mers: " << kmers.size() << "\n";
    out << "load factor: " << loadFactor << "\n";
    out << "hash functions: " << hashCount << "\n";
    out << "true positives: " << truePositives << "\n";
    out << "false positives: " << falsePositives << "\n";
    out << "false positive rate [%]: " << falsePositiveRate << "\n";

    out.close();

    // print summary to console
    std::cout << "insert time: " << insertTime << " ms\n";
    std::cout << "lookup time: " << lookupTime << " ms\n";
    std::cout << "load factor: " << loadFactor << "\n";
    std::cout << "true positives: " << truePositives << "\n";
    std::cout << "false positives: " << falsePositives << "\n";
    std::cout << "false positive rate: " << falsePositiveRate << "%\n";
    std::cout << "results saved to: " << outputFile << "\n";

    return 0;
}