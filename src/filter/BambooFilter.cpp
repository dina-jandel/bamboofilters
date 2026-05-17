// this code was written by team member: Dina Janđel, and colleague Marija Špoljarić worked on improvements and corrections
#include "BambooFilter.hpp"

#include <functional> // std::hash
#include <iostream>   // printing messages during resize
#include <random>     // random selection of buckets during cuckoo relocation

/*
briefly what the code does:
    bamboo filter: a probabilistic data structure similar to bloom filter and cuckoo filter
    used for:
    - checking whether an element exists in a set
    - very fast searching
    - low memory consumption
    - working with large datasets (dna k-mers)

    in short logic:
    - hashes dna k-mer
    - generates fingerprint
    - stores it in one of two buckets
    - if buckets are full → cuckoo relocation
    - if that also fails → bamboo resize
    - contains() checks whether the fingerprint exists in the filter
*/

// constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BambooFilter::BambooFilter(
    size_t initialBuckets,
    size_t bucketSize,
    size_t maxKicks)

    // initialization of class attributes
    : bucketCount_(initialBuckets), // number of buckets in the current table
      oldBucketCount_(0),           // number of buckets in the old table
      bucketSize_(bucketSize),      // number of fingerprint slots per bucket
      maxKicks_(maxKicks),          // maximum number of relocations
      itemCount_(0),                // number of inserted elements
      resizing_(false),             // indicates whether resizing is in progress
      migrationIndex_(0),           // current migration position
      resizeCooldown_(0),           // prevents too frequent resizing
      gen_(std::random_device{}())  // random number generator
{
    // reserve memory for buckets
    currentTable_.reserve(bucketCount_);

    // create all buckets
    for (size_t i = 0; i < bucketCount_; i++)
    {
        currentTable_.emplace_back(bucketSize_);
    }
}

// hashing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint64_t BambooFilter::hash(
    const std::string &item) const
{
    // hashes a string using std::hash
    return std::hash<std::string>{}(item);
}

uint16_t BambooFilter::fingerprint(
    const std::string &item) const
{
    // generates a shorter fingerprint from the full hash
    uint64_t h = hash(item);

    // bit mixing for better distribution
    uint16_t fp = static_cast<uint16_t>(
        (h ^ (h >> 16)) & 0xFFFF);

    // fingerprint must not be 0 because 0 is used to mark empty slots
    return (fp == 0) ? 1 : fp;
}

// index calculation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t BambooFilter::indexHash(
    uint64_t hashValue,
    size_t tableSize) const
{
    // determines primary bucket
    return hashValue % tableSize;
}

size_t BambooFilter::altIndex(
    size_t index,
    uint16_t fp,
    size_t tableSize) const
{
    // calculates alternative bucket
    // uses fingerprint and xor
    uint64_t mix = std::hash<uint32_t>{}(fp * 2654435761u);

    return (index ^ mix) % tableSize;
}

// bucket operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BambooFilter::insertIntoBucket(
    std::vector<Bucket> &table,
    size_t index,
    uint16_t fp,
    uint64_t hashValue)
{
    // iterate through all bucket slots
    for (size_t i = 0; i < bucketSize_; i++)
    {
        // if slot is empty
        if (table[index].fingerprints[i] == 0)
        {
            // store fingerprint
            table[index].fingerprints[i] = fp;
            table[index].originalHashes[i] = hashValue;
            return true;
        }
    }

    // if bucket is full:
    return false;
}

bool BambooFilter::insertIntoTable(
    std::vector<Bucket> &table,
    size_t tableSize,
    uint16_t fp,
    uint64_t hashValue)
{
    // compute two possible buckets
    size_t i1 = indexHash(hashValue, tableSize);
    size_t i2 = altIndex(i1, fp, tableSize);

    // try inserting into first bucket
    if (insertIntoBucket(table, i1, fp, hashValue))
        return true;

    // try inserting into second bucket
    if (insertIntoBucket(table, i2, fp, hashValue))
        return true;

    return false;
}

// bamboo resize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BambooFilter::startResize()
{
    if (resizing_)
        return;

    if (!oldTable_.empty())
        return;

    if (currentTable_.empty())
        return;

    // current table becomes old table
    oldTable_ = currentTable_;
    oldBucketCount_ = bucketCount_;

    // double number of buckets
    bucketCount_ *= 2;

    currentTable_.clear();
    currentTable_.reserve(bucketCount_);

    // create new larger table
    for (size_t i = 0; i < bucketCount_; i++)
    {
        currentTable_.emplace_back(bucketSize_);
    }

    resizing_ = true;
    migrationIndex_ = 0;

    std::cout << "[BambooFilter] resize started\n";
}

void BambooFilter::migrateStep()
{
    if (!resizing_)
        return;

    // resize finished
    if (migrationIndex_ >= oldTable_.size())
    {
        resizing_ = false;
        oldTable_.clear();
        migrationIndex_ = 0;

        std::cout << "[BambooFilter] resize finished\n";
        return;
    }

    // take one bucket from old table
    Bucket &bucket = oldTable_[migrationIndex_];

    for (size_t i = 0; i < bucketSize_; i++)
    {
        uint16_t fp = bucket.fingerprints[i];
        uint64_t h = bucket.originalHashes[i];

        if (fp != 0)
        {
            // move fingerprint to new table
            bool ok = insertIntoTable(currentTable_, bucketCount_, fp, h);
            if (!ok)
            {
                // fallback: try cuckoo insert (minimal)
                insertIntoBucket(currentTable_, indexHash(h, bucketCount_), fp, h);
            }
        }
    }

    migrationIndex_++;
}

// public functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BambooFilter::insert(const std::string &item)
{
    // if table is almost full -> resize
    if (loadFactor() > 0.85 && !resizing_ && resizeCooldown_ == 0)
    {
        startResize();
        resizeCooldown_ = 10 * bucketSize_;
    }

    // during resize we migrate part of the data
    if (resizing_)
    {
        migrateStep();
    }

    uint16_t fp = fingerprint(item);
    uint64_t h = hash(item);

    size_t i1 = indexHash(h, bucketCount_);
    size_t i2 = altIndex(i1, fp, bucketCount_);

    // try normal insertion
    if (insertIntoBucket(currentTable_, i1, fp, h))
    {
        itemCount_++;
        return true;
    }

    if (insertIntoBucket(currentTable_, i2, fp, h))
    {
        itemCount_++;
        return true;
    }

    /*
        cuckoo relocation:
        if both buckets are full:
        - evict existing fingerprint
        - move it to alternative location
    */

    std::uniform_int_distribution<int> coin(0, 1);

    size_t currentIndex =
        coin(gen_) == 0 ? i1 : i2;

    uint16_t currentFp = fp;
    uint64_t currentHash = h; // tracking hash of the element currently in hand

    for (size_t kick = 0; kick < maxKicks_; kick++)
    {
        std::uniform_int_distribution<size_t> slotDist(
            0,
            bucketSize_ - 1);

        size_t slot = slotDist(gen_);

        uint16_t evictedFp = currentTable_[currentIndex].fingerprints[slot];
        uint64_t evictedHash = currentTable_[currentIndex].originalHashes[slot];

        currentTable_[currentIndex].fingerprints[slot] = currentFp;
        currentTable_[currentIndex].originalHashes[slot] = currentHash;

        currentFp = evictedFp;
        currentHash = evictedHash;

        currentIndex = altIndex(indexHash(currentHash, bucketCount_), currentFp, bucketCount_);

        if (insertIntoBucket(currentTable_, currentIndex, currentFp, currentHash))
        {
            itemCount_++;
            return true;
        }
    }

    // if cuckoo fails
    if (insertIntoBucket(currentTable_, indexHash(currentHash, bucketCount_), currentFp, currentHash))
    {
        itemCount_++;
        return true;
    }

    if (!resizing_)
        startResize();

    migrateStep();

    if (resizeCooldown_ > 0)
        resizeCooldown_--;

    return false;
}

// function for checking whether a k-mer exists in the genome
bool BambooFilter::contains(
    const std::string &item) const
{
    uint64_t h = hash(item);
    uint16_t fp = fingerprint(item);

    auto searchTable =
        [&](const std::vector<Bucket> &table,
            size_t tableSize)
    {
        uint64_t hashValue = h;

        size_t i1 = indexHash(hashValue, tableSize);
        size_t i2 = altIndex(i1, fp, tableSize);

        // search first bucket
        for (size_t i = 0; i < bucketSize_; i++)
        {
            if (table[i1].fingerprints[i] == fp)
                return true;
        }

        // search second bucket
        for (size_t i = 0; i < bucketSize_; i++)
        {
            if (table[i2].fingerprints[i] == fp)
                return true;
        }

        return false;
    };

    if (searchTable(currentTable_, bucketCount_))
        return true;

    if (resizing_)
        return searchTable(oldTable_, oldBucketCount_);

    return false;
}

// statistics
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t BambooFilter::size() const
{
    return itemCount_;
}

double BambooFilter::loadFactor() const
{
    return static_cast<double>(itemCount_) /
           (bucketCount_ * bucketSize_);
}