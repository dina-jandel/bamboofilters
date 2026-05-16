// ovaj kod napisala članica tima: Marija Špoljarić
#include "DNAGenerator.hpp" // header s deklaracijama funkcija
#include <random> 
#include <string>
#include <cstdlib> // za fju rand()


std::string generateRandomDNA(int k) { // fja generira nasumicnu DNA sekvencu duljine k 
    static const char bases[4] = {'A', 'C', 'G', 'T'};  // baze adenin citozin gvanin timin

    std::string result; // string za pohranu gen. dna sekvence
    result.reserve(k); // unaprijed rezervira memoriju za k znakova, poboljsava performanse

    for (int i = 0; i < k; i++) {  // k puta generira (nasumicno) jednu od baza i pohranjuje u string result
        result.push_back(bases[rand() % 4]);   //rand()%4 daje broj 0, 1, 2 ili 3 (index polja u kojem su pohranjene baze)
    }

    return result; // pov.vr. -> slucajno generirana DNA sekvenca
}


std::string generateDNA(size_t n) {  // fja generira nasumicnu DNA sekvencu duljine n
    const char bases[4] = {'A', 'C', 'G', 'T'}; // baze 
    std::string result;
    result.reserve(n); // unaprijed rezerv. za n znakova

    //random_device generira seed za genrator slucajnih brojeva (seed sluzi za razlicite rezultate pri svakom pokretanju programa)
    std::random_device rd;
    // Mersenne Twister geenrator sluc. brojeva
    std::mt19937 gen(rd());
    // generator ima ravnomjeran odabir broja izmedu 0 i 3 (svaka baza ima jednaku vjerojatnost pojavljivanja)
    std::uniform_int_distribution<> dist(0, 3);

    for (size_t i = 0; i < n; i++)
    { 
        result += bases[dist(gen)];   // dist(gen) -> generiranje broja 0, 1, 2 ili 3
    }

    return result; // pov.vr. -> slucajno generirana DNA sekvenca
}
