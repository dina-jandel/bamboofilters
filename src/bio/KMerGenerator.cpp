// ovaj kod napisala clanica tima: Marija Špoljarić
#include "KMerGenerator.hpp" 


// (k-mer: podniz DNA sekvence duljine k)
/* pr.:
    DNA sekvenca: ACGTAG
    k = 3

    k-meri:
    ACG
    CGT
    GTA
    TAG
*/

// fja koja generira sve moguce k-mere za neku DNA sekvencu 
std::vector<std::string> generateKmers(const std::string &dna, int k)
{
    // vektor stringova koji ce spremati sve generirane k-mere
    std::vector<std::string> kmers;

    for (size_t i = 0; i + k <= dna.size(); i++)
    {
        // uzima podstring koji pocinje na indeksu i i ima duljinu k znakova
        // dodaje taj k-mer u vektor
        kmers.push_back(dna.substr(i, k));
    }

    return kmers;
}
