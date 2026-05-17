// this code was written by team member: Dina Janđel

#ifndef BAMBOO_FILTER_HPP
#define BAMBOO_FILTER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <random>

// file with declarations for the BambooFilter class

class BambooFilter
{
public:
    /*
        class constructor

        initialBuckets -> initial number of buckets
        bucketSize     -> number of slots per bucket
        maxKicks       -> maximum number of cuckoo relocations
    */
    BambooFilter(
        size_t initialBuckets = 100000,
        size_t bucketSize = 4,
        size_t maxKicks = 500);

    /*
        inserts a new element into the filter

        returns:
        true  (successful insertion)
        false (insertion failed)
    */
    bool insert(const std::string &item);

    /*
        checks whether an element exists in the filter

        returns:
        true  (element may exist)
        false (element definitely does not exist)
    */
    bool contains(const std::string &item) const;

    // returns total number of stored elements

    size_t size() const;

    // calculates filter load factor

    double loadFactor() const;

private:
    /*
        bucket -> one block in the table
        each bucket contains multiple fingerprints
        fingerprint -> shortened version of a hash value
    */
    struct Bucket
    {
        std::vector<uint16_t> fingerprints;
        std::vector<uint64_t> originalHashes; // Dodano polje za pohranu originalnog hasha radi ispravne migracije bez false negatives

        /*
            bucket constructor

            creates a bucket of a given size and initializes all slots to 0 (0 indicates an empty slot)
        */
        explicit Bucket(size_t bucketSize)
            : fingerprints(bucketSize, 0),
              originalHashes(bucketSize, 0) {} // Inicijalizacija vektora hasheva na 0
    };

    // TABLES

    // current active table
    std::vector<Bucket> currentTable_;

    // old table during resize process
    std::vector<Bucket> oldTable_;

    // FILTER PARAMETERS

    size_t bucketCount_;    // number of buckets in current table
    size_t oldBucketCount_; // number of buckets in old table
    size_t bucketSize_;     // number of slots per bucket
    size_t maxKicks_;       // maximum number of relocations
    size_t itemCount_;      // number of inserted elements

    // random number generator for cuckoo relocation
    std::mt19937 gen_;

    // BAMBOO RESIZE VARIABLES

    bool resizing_;         // indicates whether resizing is in progress
    size_t migrationIndex_; // current migration position
    size_t resizeCooldown_; // prevents too frequent resizing

    // HASH FUNCTIONS

    // generates a hash value for a given string
    uint64_t hash(const std::string &item) const;

    // generates a shorter fingerprint from a hash value
    uint16_t fingerprint(const std::string &item) const;

    // INDEX CALCULATION

    // computes the first bucket index
    size_t indexHash(
        uint64_t hashValue,
        size_t tableSize) const;

    // computes the alternative bucket index
    size_t altIndex(
        size_t index,
        uint16_t fp,
        size_t tableSize) const;

    // INSERTION OPERATIONS

    // tries to insert a fingerprint into a specific bucket

    bool insertIntoBucket(
        std::vector<Bucket> &table,
        size_t index,
        uint16_t fp,
        uint64_t hashValue); // Ažurirano da prima hashValue umjesto stringa

    // tries to insert a fingerprint into the whole table
    bool insertIntoTable(
        std::vector<Bucket> &table,
        size_t tableSize,
        uint16_t fp,
        uint64_t hashValue); // Promijenjeno iz const std::string &item u uint64_t hashValue

    // RESIZE OPERATIONS

    // starts Bamboo resize - creates a new larger table
    void startResize();

    // gradually migrates data from the old table to the new one
    void migrateStep();
};

#endif