//ovaj kod napisala je clanica tima: Dina Janđel, a kolegica Marija Špoljarić je radila na poboljsanju i ispravljanju
#include "BambooFilter.hpp"

#include <functional>   // std::hash
#include <iostream>     // ispis poruka tijekom resize-a
#include <random>       // random odabir bucketa kod cuckoo relokacije

/*
ukratko sto kod radi:
    Bamboo filter: probabilistička struktura podataka slična Bloom filteru i Cuckoo filteru
    koristi se za:
    - provjeru postoji li element u skupu
    - vrlo brzo pretraživanje
    - manju potrošnju memorije
    - rad s velikim datasetovima (DNA k-meri)

    ukratko logika:
    - hashira DNA k-mer
    - generira fingerprint
    - sprema ga u jedan od dva bucketa
    - ako su bucketi puni → cuckoo relocation
    - ako ni to ne uspije → Bamboo resize
    - contains() provjerava postoji li fingerprint u filteru
*/

// KONSTRUKTOR
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BambooFilter::BambooFilter(
    size_t initialBuckets,
    size_t bucketSize,
    size_t maxKicks)

    // inicijalizacija atributa klase
    : bucketCount_(initialBuckets),   // broj bucketa u trenutnoj tablici
      oldBucketCount_(0),             // broj bucketa stare tablice
      bucketSize_(bucketSize),        // broj fingerprint slotova po bucketu
      maxKicks_(maxKicks),            // maksimalan broj relokacija
      itemCount_(0),                  // broj umetnutih elemenata
      resizing_(false),               // oznacava traje li resize
      migrationIndex_(0),             // trenutna pozicija migracije
      resizeCooldown_(0),             // sprjecava precesto resizeanje
      gen_(std::random_device{}())    // generator slucajnih brojeva
{
    // rezervacija memorije za buckete
    currentTable_.reserve(bucketCount_);

    // kreiranje svih bucketa
    for (size_t i = 0; i < bucketCount_; i++)
    {
        currentTable_.emplace_back(bucketSize_);
    }
}


// HASHIRANJE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint64_t BambooFilter::hash(
    const std::string &item) const
{
    // hashira string pomoću std::hash
    return std::hash<std::string>{}(item);
}

uint16_t BambooFilter::fingerprint(
    const std::string &item) const
{
    // generira kraci fingerprint iz punog hash-a
    uint64_t h = hash(item);

    // mijesanje bitova radi bolje distribucije
    uint16_t fp = static_cast<uint16_t>(
        (h ^ (h >> 16)) & 0xFFFF);

    // fingerprint ne smije biti 0 jer se 0 koristi za oznacavanje praznog mjesta
    return (fp == 0) ? 1 : fp;
}


// IZRACUN INDEKSA
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t BambooFilter::indexHash(
    uint64_t hashValue,
    size_t tableSize) const
{
    // određuje primarni bucket
    return hashValue % tableSize;
}

size_t BambooFilter::altIndex(
    size_t index,
    uint16_t fp,
    size_t tableSize) const
{
    // racuna alternativni bucket
    // koristi fingerprint i XOR 
    uint64_t mix = std::hash<uint32_t>{}(fp * 2654435761u);

    return (index ^ mix) % tableSize;
}

// OPERACIJE NAD BUCKETIMA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BambooFilter::insertIntoBucket(
    std::vector<Bucket> &table,
    size_t index,
    uint16_t fp)
{
    // prolaz kroz sve slotove bucketa
    for (size_t i = 0; i < bucketSize_; i++)
    {
        // ako je slot prazan
        if (table[index].fingerprints[i] == 0)
        {
            // spremi fingerprint
            table[index].fingerprints[i] = fp;
            return true;
        }
    }

    // ako je bucket pun:
    return false;
}

bool BambooFilter::insertIntoTable(
    std::vector<Bucket> &table,
    size_t tableSize,
    uint16_t fp,
    const std::string &item)
{
    uint64_t h = hash(item);

    // izracun dva moguca bucketa
    size_t i1 = indexHash(h, tableSize);
    size_t i2 = altIndex(i1, fp, tableSize);

    // pokusaj umetanja u prvi bucket 
    if (insertIntoBucket(table, i1, fp))
        return true;

    // pokusaj umetanja u drugi bucket
    if (insertIntoBucket(table, i2, fp))
        return true;

    return false;
}

// BAMBOO RESIZE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BambooFilter::startResize()
{
    if (resizing_)
        return;

    if (!oldTable_.empty())
        return;

    if (currentTable_.empty())
        return;

    // trenutna tablica postaje stara tablica
    oldTable_ = currentTable_;
    oldBucketCount_ = bucketCount_;

    // udvostrucuje broj bucketa
    bucketCount_ *= 2;

    currentTable_.clear();
    currentTable_.reserve(bucketCount_);

    // kreira novu vecu tablicu
    for (size_t i = 0; i < bucketCount_; i++)
    {
        currentTable_.emplace_back(bucketSize_);
    }

    resizing_ = true;
    migrationIndex_ = 0;

    std::cout << "[BambooFilter] Resize started\n";
}

void BambooFilter::migrateStep()
{
    if (!resizing_)
        return;

    // migracija zavrsena
    if (migrationIndex_ >= oldTable_.size())
    {
        resizing_ = false;
        oldTable_.clear();
        migrationIndex_ = 0;

        std::cout << "[BambooFilter] Resize finished\n";
        return;
    }

    // uzima jedan bucket iz stare tablice
    Bucket &bucket = oldTable_[migrationIndex_];

    for (uint16_t fp : bucket.fingerprints)
    {
        if (fp != 0)
        {
            // premjesta fingerprint u novu tablicu
            insertIntoTable(
                currentTable_,
                bucketCount_,
                fp,
                "");
        }
    }

    migrationIndex_++;
}

// JAVNE FUNKCIJE 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BambooFilter::insert(const std::string &item)
{
    // ako je tablica skoro puna -> resize
    if (loadFactor() > 0.85 && !resizing_ && resizeCooldown_ == 0)
    {
        startResize();
        resizeCooldown_ = 10 * bucketSize_;
    }

    // tijekom resize-a migriramo dio podataka
    if (resizing_)
    {
        migrateStep();
    }

    uint16_t fp = fingerprint(item);
    uint64_t h = hash(item);

    size_t i1 = indexHash(h, bucketCount_);
    size_t i2 = altIndex(i1, fp, bucketCount_);

    // pokusaj normalnog umetanja
    if (insertIntoBucket(currentTable_, i1, fp))
    {
        itemCount_++;
        return true;
    }

    if (insertIntoBucket(currentTable_, i2, fp))
    {
        itemCount_++;
        return true;
    }

    /*
        CUCKOO RELOCATION
        ako su oba bucketa puna:
        - izbacujemo postojeci fingerprint
        - premjestamo ga na alternativnu lokaciju
    */

    std::uniform_int_distribution<int> coin(0, 1);

    size_t currentIndex =
        coin(gen_) == 0 ? i1 : i2;

    uint16_t currentFp = fp;

    for (size_t kick = 0; kick < maxKicks_; kick++)
    {
        std::uniform_int_distribution<size_t> slotDist(
            0,
            bucketSize_ - 1);

        size_t slot = slotDist(gen_);

        // zamjena fingerprinta
        std::swap(
            currentFp,
            currentTable_[currentIndex].fingerprints[slot]);

        // novi alternativni bucket
        currentIndex =
            altIndex(
                currentIndex,
                currentFp,
                bucketCount_);

        // pokusaj umetanja
        if (insertIntoBucket(
                currentTable_,
                currentIndex,
                currentFp))
        {
            itemCount_++;
            return true;
        }
    }

    /*
        ako relokacija ne uspije:
        pokrece se Bamboo resize
    */
    if (!resizing_)
        startResize();

    migrateStep();

    if (resizeCooldown_ > 0)
        resizeCooldown_--;

    return false;
}

//fja za provjeru postoji li k-mer u genomu
bool BambooFilter::contains(
    const std::string &item) const
{
    uint16_t fp = fingerprint(item);

    auto searchTable =
        [&](const std::vector<Bucket> &table,
            size_t tableSize)
    {
        uint64_t h = hash(item);

        size_t i1 = indexHash(h, tableSize);
        size_t i2 = altIndex(i1, fp, tableSize);

        // pretraga prvog bucketa
        for (auto v : table[i1].fingerprints)
            if (v == fp)
                return true;

        // pretraga drugog bucketa
        for (auto v : table[i2].fingerprints)
            if (v == fp)
                return true;

        return false;
    };

    // provjera u trenutnoj tablici
    if (searchTable(currentTable_, bucketCount_))
        return true;

    // tijekom resize-a provjeri i staru tablicu
    // (ako je resize u tijeku, dio podataka je možda još u staroj tablici, zato mora provjeriti i novu i staru) 
    if (resizing_)
    {
        return searchTable(oldTable_, oldBucketCount_);
    }

    return false;
}


// STATISTIKE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t BambooFilter::size() const
{
    // vraca broj spremljenih elemenata
    return itemCount_;
}

double BambooFilter::loadFactor() const
{
    // racuna popunjenost filtera
    return static_cast<double>(itemCount_) /
           (bucketCount_ * bucketSize_);
}
