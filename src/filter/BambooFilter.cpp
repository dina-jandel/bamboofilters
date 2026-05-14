#include "BambooFilter.hpp"

#include <functional>
#include <iostream>
#include <random>

BambooFilter::BambooFilter(
    size_t initialBuckets,
    size_t bucketSize,
    size_t maxKicks)
    : bucketCount_(initialBuckets),
      oldBucketCount_(0),
      bucketSize_(bucketSize),
      maxKicks_(maxKicks),
      itemCount_(0),
      resizing_(false),
      migrationIndex_(0),
      resizeCooldown_(0),
      gen_(std::random_device{}())
{

    currentTable_.reserve(bucketCount_);

    for (size_t i = 0; i < bucketCount_; i++)
    {
        currentTable_.emplace_back(bucketSize_);
    }
}

/* ---------------- HASHING ---------------- */

uint64_t BambooFilter::hash(
    const std::string &item) const
{

    return std::hash<std::string>{}(item);
}

uint16_t BambooFilter::fingerprint(
    const std::string &item) const
{

    // better entropy than raw lower 16 bits only
    uint64_t h = hash(item);

    uint16_t fp = static_cast<uint16_t>(
        (h ^ (h >> 16)) & 0xFFFF);

    return (fp == 0) ? 1 : fp;
}

/* ---------------- INDEXING ---------------- */

size_t BambooFilter::indexHash(
    uint64_t hashValue,
    size_t tableSize) const
{

    return hashValue % tableSize;
}

size_t BambooFilter::altIndex(
    size_t index,
    uint16_t fp,
    size_t tableSize) const
{

    // stronger mixing than simple XOR(fp)
    uint64_t mix = std::hash<uint32_t>{}(fp * 2654435761u);

    return (index ^ mix) % tableSize;
}

/* ---------------- BUCKET OPS ---------------- */

bool BambooFilter::insertIntoBucket(
    std::vector<Bucket> &table,
    size_t index,
    uint16_t fp)
{

    for (size_t i = 0; i < bucketSize_; i++)
    {
        if (table[index].fingerprints[i] == 0)
        {
            table[index].fingerprints[i] = fp;
            return true;
        }
    }

    return false;
}

bool BambooFilter::insertIntoTable(
    std::vector<Bucket> &table,
    size_t tableSize,
    uint16_t fp,
    const std::string &item)
{

    uint64_t h = hash(item);

    size_t i1 = indexHash(h, tableSize);
    size_t i2 = altIndex(i1, fp, tableSize);

    if (insertIntoBucket(table, i1, fp))
        return true;
    if (insertIntoBucket(table, i2, fp))
        return true;

    return false;
}

/* ---------------- RESIZE (BAMBOO STYLE) ---------------- */

void BambooFilter::startResize()
{
    if (resizing_)
        return;
    if (!oldTable_.empty())
        return;
    if (currentTable_.empty())
        return;

    oldTable_ = currentTable_;
    oldBucketCount_ = bucketCount_;

    bucketCount_ *= 2;

    currentTable_.clear();
    currentTable_.reserve(bucketCount_);

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

    if (migrationIndex_ >= oldTable_.size())
    {
        resizing_ = false;
        oldTable_.clear();
        migrationIndex_ = 0;
        std::cout << "[BambooFilter] Resize finished\n";
        return;
    }

    Bucket &bucket = oldTable_[migrationIndex_];

    for (uint16_t fp : bucket.fingerprints)
    {
        if (fp != 0)
        {
            insertIntoTable(
                currentTable_,
                bucketCount_,
                fp,
                "");
        }
    }

    migrationIndex_++;
}

/* ---------------- PUBLIC API ---------------- */

bool BambooFilter::insert(const std::string &item)
{
    if (loadFactor() > 0.85 && !resizing_ && resizeCooldown_ == 0)
    {
        startResize();
        resizeCooldown_ = 10 * bucketSize_;
    }

    if (resizing_)
    {
        migrateStep();
    }

    uint16_t fp = fingerprint(item);
    uint64_t h = hash(item);

    size_t i1 = indexHash(h, bucketCount_);
    size_t i2 = altIndex(i1, fp, bucketCount_);

    // Try normal insertion first
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
     * Cuckoo relocation:
     * randomly evict existing fingerprints
     * and try alternate positions
     */
    std::uniform_int_distribution<int> coin(0, 1);
    size_t currentIndex = coin(gen_) == 0 ? i1 : i2;
    uint16_t currentFp = fp;

    for (size_t kick = 0; kick < maxKicks_; kick++)
    {
        std::uniform_int_distribution<size_t> slotDist(0, bucketSize_ - 1);
        size_t slot = slotDist(gen_);

        std::swap(
            currentFp,
            currentTable_[currentIndex].fingerprints[slot]);

        currentIndex =
            altIndex(
                currentIndex,
                currentFp,
                bucketCount_);

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
     * If relocation fails:
     * trigger Bamboo resize
     */
    if (!resizing_)
        startResize();

    migrateStep();

    if (resizeCooldown_ > 0)
        resizeCooldown_--;

    return false;
}

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

        for (auto v : table[i1].fingerprints)
            if (v == fp)
                return true;

        for (auto v : table[i2].fingerprints)
            if (v == fp)
                return true;

        return false;
    };

    if (searchTable(currentTable_, bucketCount_))
        return true;

    if (resizing_)
    {
        return searchTable(oldTable_, oldBucketCount_);
    }

    return false;
}

/* ---------------- STATS ---------------- */

size_t BambooFilter::size() const
{
    return itemCount_;
}

double BambooFilter::loadFactor() const
{
    return static_cast<double>(itemCount_) /
           (bucketCount_ * bucketSize_);
}
