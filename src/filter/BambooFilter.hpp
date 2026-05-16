// ovaj kod napisala je clanica tima: Dina Janđel

#ifndef BAMBOO_FILTER_HPP
#define BAMBOO_FILTER_HPP

#include <cstdint>   
#include <string>    
#include <vector>    
#include <random>    


//datoteka s deklaracijama za BambooFilter klasu


class BambooFilter
{
public:

    /*
        konstruktor klase

        initialBuckets -> pocetni broj bucketa
        bucketSize     -> broj slotova po bucketu
        maxKicks       -> maksimalan broj cuckoo relokacija
    */
    BambooFilter(
        size_t initialBuckets = 100000,
        size_t bucketSize = 4,
        size_t maxKicks = 500);

    /*
        umece novi element u filter

        vraca:
        true  (uspjesno umetanje)
        false (umetanje nije uspjelo)
    */
    bool insert(const std::string &item);

    /*
        provjerava postoji li element u filteru

        vraca:
        true  (element mozda postoji)
        false (element sigurno ne postoji)
    */
    bool contains(const std::string &item) const;

    
    //vraca ukupan broj spremljenih elemenata
    
    size_t size() const;

    
    //racuna popunjenost filtera

    double loadFactor() const;

private:

    /*
        bucket -> jedan blok u tablici
        svaki bucket sadrzi vise fingerprintova
        fingerprint -> skracena verzija hash vrijednosti
    */
    struct Bucket
    {
        std::vector<uint16_t> fingerprints;

        /*
            konstruktor bucketa

            kreira bucket odredene velicine i sva mjesta inicijalizira na 0 (0 oznacava prazan slot)
        */
        explicit Bucket(size_t bucketSize)
            : fingerprints(bucketSize, 0) {}
    };


    // TABLICE

    // trenutna aktivna tablica
    std::vector<Bucket> currentTable_;

    // stara tablica tijekom resize procesa
    std::vector<Bucket> oldTable_;


    // PARAMETRI FILTERA

    size_t bucketCount_;      // broj bucketa trenutne tablice
    size_t oldBucketCount_;   // broj bucketa stare tablice
    size_t bucketSize_;       // broj slotova po bucketu
    size_t maxKicks_;         // maksimalan broj relokacija
    size_t itemCount_;        // broj umetnutih elemenata

    // generator slucajnih brojeva za cuckoo relocation
    std::mt19937 gen_;

    // BAMBOO RESIZE VARIJABLE

    bool resizing_;           // oznacava traje li resize
    size_t migrationIndex_;   // trenutna pozicija migracije
    size_t resizeCooldown_;   // sprjecava precesto resizeanje

    // HASH FUNKCIJE

    // generira hash vrijednost za zadani string
    uint64_t hash(const std::string &item) const;

    // gnerira kraci fingerprint iz hash vrijednosti
    uint16_t fingerprint(const std::string &item) const;

    // IZRACUN INDEKSA

    // racuna prvi bucket indeks
    size_t indexHash(
        uint64_t hashValue,
        size_t tableSize) const;

    // racuna alternativni bucket indeks
    size_t altIndex(
        size_t index,
        uint16_t fp,
        size_t tableSize) const;


    // OPERACIJE UMETANJA

    // pokusava umetnuti fingerprint u određeni bucket

    bool insertIntoBucket(
        std::vector<Bucket> &table,
        size_t index,
        uint16_t fp);

    // pokusava umetnuti fingerprint u cijelu tablicu
    bool insertIntoTable(
        std::vector<Bucket> &table,
        size_t tableSize,
        uint16_t fp,
        const std::string &item);


    // RESIZE OPERACIJE

    // pokrece Bamboo resize - stvara novu vecu tablicu
    void startResize();

    // postepeno migrira podatke iz stare tablice u novu
    void migrateStep();
};

#endif
