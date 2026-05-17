#pragma once
// this code was written by team member: Marija Špoljarić
#include <vector>
#include <string>
#include <functional>
#include <cstdint>

// bloom filter: probabilistic data structure for set membership testing
// used for:
// - fast membership queries
// - memory-efficient storage
// - allowing false positives but no false negatives

class BloomFilter
{
public:
    // constructor
    BloomFilter(size_t capacity = 1000000);

    // inserts an item into the filter
    void insert(const std::string &item);

    // checks whether an item might exist in the filter
    bool contains(const std::string &item) const;

private:
    // bit array representing the filter
    std::vector<uint8_t> bitArray;

    // total size of the bit array
    size_t capacity_;

    // hash functions used to map items into bit positions
    size_t hash1(const std::string &item) const;
    size_t hash2(const std::string &item) const;
    size_t hash3(const std::string &item) const;
};