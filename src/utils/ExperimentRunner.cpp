#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "../filter/BambooFilter.hpp"
#include "../filter/BloomFilter.hpp"
#include "../bio/DNAGenerator.hpp"
#include "../bio/KMerGenerator.hpp"
#include "Benchmark.hpp"
#include "FASTAParser.hpp"

void runExperiments(
    const std::string &outputFile,
    const std::string &fastaFile)
{
    std::ofstream out(outputFile);

    std::vector<int> kValues = {10, 20, 50, 100, 200};
    std::vector<size_t> dnaSizes = {100000, 500000, 1000000};

    for (size_t dnaSize : dnaSizes)
    {
        std::string dna;

        if (fastaFile.empty())
        {
            dna = generateDNA(dnaSize);
            std::cout << "using synthetic dna\n";
        }
        else
        {
            dna = readFasta(fastaFile);
            std::cout << "using fasta file\n";
        }

        if (dna.empty())
            continue;

        std::cout << "dna size: " << dna.size() << "\n";

        for (int k : kValues)
        {
            if (k >= (int)dna.size())
                continue;

            auto kmers = generateKmers(dna, k);

            BambooFilter bamboo(20000);

            Benchmark t1;
            t1.start();
            for (const auto &kmer : kmers)
                bamboo.insert(kmer);
            long long bambooInsert = t1.stop();

            Benchmark t2;
            t2.start();
            int bambooTP = 0;
            for (const auto &kmer : kmers)
                if (bamboo.contains(kmer))
                    bambooTP++;
            long long bambooLookup = t2.stop();

            int bambooFP = 0;
            int testSamples = 100000;

            for (int i = 0; i < testSamples; i++)
            {
                std::string fake = generateDNA(k);
                if (bamboo.contains(fake))
                    bambooFP++;
            }

            double bambooFPRate = (double)bambooFP / testSamples * 100.0;

            BloomFilter bloom(20000);

            Benchmark t3;
            t3.start();
            for (const auto &kmer : kmers)
                bloom.insert(kmer);
            long long bloomInsert = t3.stop();

            Benchmark t4;
            t4.start();
            int bloomTP = 0;
            for (const auto &kmer : kmers)
                if (bloom.contains(kmer))
                    bloomTP++;
            long long bloomLookup = t4.stop();

            int bloomFP = 0;
            for (int i = 0; i < testSamples; i++)
            {
                std::string fake = generateDNA(k);
                if (bloom.contains(fake))
                    bloomFP++;
            }

            double bloomFPRate = (double)bloomFP / testSamples * 100.0;

            out << "DNA size: " << dna.size() << "\n";
            out << "k: " << k << "\n";
            out << "kmers: " << kmers.size() << "\n\n";

            out << "--- BAMBOO FILTER ---\n";
            out << "insert time [ms]: " << bambooInsert << "\n";
            out << "lookup time [ms]: " << bambooLookup << "\n";
            out << "false positive rate [%]: " << bambooFPRate << "\n\n";

            out << "--- BLOOM FILTER ---\n";
            out << "insert time [ms]: " << bloomInsert << "\n";
            out << "lookup time [ms]: " << bloomLookup << "\n";
            out << "false positive rate [%]: " << bloomFPRate << "\n\n";

            std::cout << "done k=" << k << " dna=" << dna.size() << "\n";
        }
    }

    out.close();
}