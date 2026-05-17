// this code was written by team member: Marija Špoljarić
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

// function that runs full benchmark comparison between bamboo filter and bloom filter
void runExperiments(
    const std::string &outputFile,
    const std::string &fastaFile)
{
    // open output file where results will be written
    std::ofstream out(outputFile);

    // different k-mer sizes used for testing
    std::vector<int> kValues = {10, 20, 50, 100, 200};

    // different dna input sizes for scalability testing
    std::vector<size_t> dnaSizes = {100000, 500000, 1000000};

    // loop over different dna sizes
    for (size_t dnaSize : dnaSizes)
    {
        std::string dna;

        // if no fasta file is provided, generate synthetic dna
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

        // skip if dna is empty
        if (dna.empty())
            continue;

        std::cout << "dna size: " << dna.size() << "\n";

        // loop over different k values
        for (int k : kValues)
        {
            // skip invalid k values
            if (k >= (int)dna.size())
                continue;

            // generate all k-mers from dna sequence
            auto kmers = generateKmers(dna, k);

            // BAMBOO FILTER BENCHMARK

            // create bamboo filter with fixed capacity
            BambooFilter bamboo(20000);

            // store successfully inserted elements for validation
            std::vector<std::string> inserted;

            // start timer for insertion phase
            Benchmark t1;
            t1.start();

            // insert all k-mers into bamboo filter
            for (const auto &kmer : kmers)
            {
                if (bamboo.insert(kmer))
                    inserted.push_back(kmer);
            }

            // stop insertion timer
            long long bambooInsert = t1.stop();

            // start timer for lookup phase
            Benchmark t2;
            t2.start();

            int bambooTP = 0;

            // verify true positives using inserted elements
            for (const auto &kmer : inserted)
            {
                if (bamboo.contains(kmer))
                    bambooTP++;
            }

            // stop lookup timer
            long long bambooLookup = t2.stop();

            // compute false negatives for bamboo filter
            int bambooFN = inserted.size() - bambooTP;

            int bambooFP = 0;
            int testSamples = 100000;

            // test false positive rate using random dna strings
            for (int i = 0; i < testSamples; i++)
            {
                std::string fake = generateDNA(k);

                if (bamboo.contains(fake))
                    bambooFP++;
            }

            // compute false positive rate in percentage
            double bambooFPRate =
                (double)bambooFP / testSamples * 100.0;

            // BLOOM FILTER BENCHMARK

            // bloom filter capacity is increased to reduce saturation
            BloomFilter bloom(kmers.size() * 10);

            // start bloom insertion timer
            Benchmark t3;
            t3.start();

            // insert all k-mers into bloom filter
            for (const auto &kmer : kmers)
                bloom.insert(kmer);

            // stop bloom insertion timer
            long long bloomInsert = t3.stop();

            // start bloom lookup timer
            Benchmark t4;
            t4.start();

            // bloom filter assumes all inserted elements are present
            int bloomTP = kmers.size();
            int bloomFN = 0;

            // stop bloom lookup timer
            long long bloomLookup = t4.stop();

            int bloomFP = 0;

            // test bloom false positives
            for (int i = 0; i < testSamples; i++)
            {
                std::string fake = generateDNA(k);

                if (bloom.contains(fake))
                    bloomFP++;
            }

            // compute bloom false positive rate
            double bloomFPRate =
                (double)bloomFP / testSamples * 100.0;

            // OUTPUT RESULTS

            // write experiment summary to file
            out << "=====================================\n";
            out << "DNA size: " << dna.size() << "\n";
            out << "k: " << k << "\n";
            out << "kmers: " << kmers.size() << "\n\n";

            // bamboo results section
            out << "--- BAMBOO FILTER ---\n";
            out << "insert time [ms]: " << bambooInsert << "\n";
            out << "lookup time [ms]: " << bambooLookup << "\n";
            out << "true positives: " << bambooTP << "\n";
            out << "false negatives: " << bambooFN << "\n";
            out << "false positive rate [%]: " << bambooFPRate << "\n";
            out << "load factor: " << bamboo.loadFactor() << "\n\n";

            // bloom results section
            out << "--- BLOOM FILTER ---\n";
            out << "insert time [ms]: " << bloomInsert << "\n";
            out << "lookup time [ms]: " << bloomLookup << "\n";
            out << "true positives: " << bloomTP << "\n";
            out << "false negatives: " << bloomFN << "\n";
            out << "false positive rate [%]: " << bloomFPRate << "\n\n";

            std::cout << "done k=" << k
                      << " dna=" << dna.size() << "\n";
        }
    }

    // close output file
    out.close();
}