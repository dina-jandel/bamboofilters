#include "BloomFilter.hpp"

BloomFilter::BloomFilter(size_t capacity)
    : bitArray(capacity, 0), capacity_(capacity) {}

size_t BloomFilter::hash1(const std::string &item) const
{
    return std::hash<std::string>{}(item);
}

size_t BloomFilter::hash2(const std::string &item) const
{
    return std::hash<std::string>{}(item + "salt1");
}

size_t BloomFilter::hash3(const std::string &item) const
{
    return std::hash<std::string>{}(item + "salt2");
}

void BloomFilter::insert(const std::string &item)
{
    bitArray[hash1(item) % capacity_] = 1;
    bitArray[hash2(item) % capacity_] = 1;
    bitArray[hash3(item) % capacity_] = 1;
}

bool BloomFilter::contains(const std::string &item) const
{
    return bitArray[hash1(item) % capacity_] &&
           bitArray[hash2(item) % capacity_] &&
           bitArray[hash3(item) % capacity_];
}