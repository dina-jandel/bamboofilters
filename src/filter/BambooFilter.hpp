#ifndef BAMBOO_FILTER_HPP
#define BAMBOO_FILTER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <random>

/*
 * Bamboo Filter (Cuckoo-style probabilistic filter)
 * - fingerprint-based storage
 * - dual-bucket insertion
 * - incremental resizing (lazy migration)
 */

class BambooFilter
{
public:
    BambooFilter(
        size_t initialBuckets = 100000,
        size_t bucketSize = 4,
        size_t maxKicks = 500);

    bool insert(const std::string &item);
    bool contains(const std::string &item) const;

    size_t size() const;
    double loadFactor() const;

private:
    struct Bucket
    {
        std::vector<uint16_t> fingerprints;

        explicit Bucket(size_t bucketSize)
            : fingerprints(bucketSize, 0) {}
    };

    /* ---------------- TABLES ---------------- */
    std::vector<Bucket> currentTable_;
    std::vector<Bucket> oldTable_;

    /* ---------------- CONFIG ---------------- */
    size_t bucketCount_;
    size_t oldBucketCount_;
    size_t bucketSize_;
    size_t maxKicks_;
    size_t itemCount_;
    std::mt19937 gen_;

    /* ---------------- RESIZE STATE ---------------- */
    bool resizing_;
    size_t migrationIndex_;
    size_t resizeCooldown_;

    /* ---------------- HASHING ---------------- */
    uint64_t hash(const std::string &item) const;
    uint16_t fingerprint(const std::string &item) const;

    size_t indexHash(
        uint64_t hashValue,
        size_t tableSize) const;

    size_t altIndex(
        size_t index,
        uint16_t fp,
        size_t tableSize) const;

    /* ---------------- CORE OPS ---------------- */
    bool insertIntoBucket(
        std::vector<Bucket> &table,
        size_t index,
        uint16_t fp);

    bool insertIntoTable(
        std::vector<Bucket> &table,
        size_t tableSize,
        uint16_t fp,
        const std::string &item);

    /* ---------------- RESIZE ---------------- */
    void startResize();
    void migrateStep();
};

#endif
