#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "../filter/BambooFilter.hpp"
#include "../bio/DNAGenerator.hpp"
#include "../bio/KMerGenerator.hpp"
#include "Benchmark.hpp"
#include "FASTAParser.hpp"

void runExperiments(
    const std::string& outputFile,
    const std::string& fastaFile) {

    std::ofstream out(outputFile);

    std::vector<int> kValues = {10, 20, 50, 100, 200};
    std::vector<size_t> dnaSizes = {100000, 500000, 1000000};

    for (size_t dnaSize : dnaSizes) {

        std::string dna;

        if (fastaFile.empty()) {
            dna = generateRandomDNA(dnaSize);
            std::cout << "using synthetic dna\n";
        } else {
            dna = readFasta(fastaFile);
            std::cout << "using fasta file\n";
        }

        if (dna.empty()) continue;

        std::cout << "dna size: " << dna.size() << "\n";

        for (int k : kValues) {

            if (k >= (int)dna.size()) continue;

            auto kmers = generateKmers(dna, k);

            BambooFilter filter(20000);

            /* ---------------- INSERT ---------------- */
            Benchmark t1;
            t1.start();

            for (const auto& kmer : kmers) {
                filter.insert(kmer);
            }

            long long insertTime = t1.stop();

            /* ---------------- LOOKUP (true positives) ---------------- */
            Benchmark t2;
            t2.start();

            int truePositives = 0;

            for (const auto& kmer : kmers) {
                if (filter.contains(kmer)) {
                    truePositives++;
                }
            }

            long long lookupTime = t2.stop();

            /* ---------------- FALSE POSITIVES ---------------- */
            int falsePositives = 0;
            int testSamples = 100000;

            for (int i = 0; i < testSamples; i++) {

                std::string fake = generateRandomDNA(k);

                if (filter.contains(fake)) {
                    falsePositives++;
                }
            }

            double fpRate =
                (double)falsePositives / testSamples * 100.0;

            double loadFactor =
                filter.loadFactor();




            

            /* ---------------- OUTPUT ---------------- */
            out << "DNA size: " << dna.size() << "\n";
            out << "k: " << k << "\n";
            out << "kmers: " << kmers.size() << "\n";
            out << "insert time: " << insertTime << " ms\n";
            out << "lookup time: " << lookupTime << " ms\n";
            out << "load factor: " << loadFactor << "\n";
            out << "true positives: " << truePositives << "\n";
            out << "false positives: " << falsePositives << "\n";
            out << "false positive rate: " << fpRate << "%\n\n";

            std::cout << "done k=" << k
                      << " dna=" << dna.size()
                      << "\n";
        }
    }

    out.close();
}
