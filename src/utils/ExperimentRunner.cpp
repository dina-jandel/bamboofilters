#include <iostream>
#include <fstream>
#include <vector>
#include "filter/BambooFilter.hpp"
#include "bio/DNAGenerator.hpp"
#include "bio/KMerGenerator.hpp"
#include "utils/Benchmark.hpp"
#include "utils/FASTAParser.hpp"

// runs a single experiment configuration
void runExperiment(const std::string &dna,
                   int k,
                   size_t size,
                   int hashes,
                   const std::string &outputName)
{
    BambooFilter bf(size, hashes);

    auto kmers = generateKmers(dna, k);

    Benchmark b1;
    b1.start();

    for (const auto &kmer : kmers)
        bf.insert(kmer);

    long long insertTime = b1.stop();

    Benchmark b2;
    b2.start();

    int tp = 0;
    for (const auto &kmer : kmers)
        if (bf.contains(kmer))
            tp++;

    long long lookupTime = b2.stop();

    int fp = 0;
    int tests = 50000;

    for (int i = 0; i < tests; i++)
    {
        std::string fake = generateDNA(k);
        if (bf.contains(fake))
            fp++;
    }

    double fpr = (double)fp / tests * 100.0;
    double load = (double)kmers.size() / size;

    std::ofstream out(outputName, std::ios::app);

    out << k << ","
        << size << ","
        << hashes << ","
        << insertTime << ","
        << lookupTime << ","
        << fpr << ","
        << load << "\n";

    out.close();

    std::cout << "done k=" << k
              << " size=" << size
              << " hashes=" << hashes << "\n";
}

// main experiment driver
int main()
{
    std::string dna = generateDNA(200000);

    std::vector<int> kValues = {10, 20, 50, 100, 200};
    std::vector<size_t> sizes = {100000, 500000, 1000000};
    std::vector<int> hashes = {3, 5, 7, 10};

    std::string output = "results.csv";

    // csv header
    std::ofstream out(output);
    out << "k,size,hashes,insert_time,lookup_time,fpr,load_factor\n";
    out.close();

    for (int k : kValues)
    {
        for (size_t s : sizes)
        {
            for (int h : hashes)
            {
                runExperiment(dna, k, s, h, output);
            }
        }
    }

    std::cout << "all experiments finished\n";

    return 0;
}