// this code was written by team member: Marija Špoljarić
#include "BloomFilter.hpp"

// constructor
BloomFilter::BloomFilter(size_t capacity)
    : bitArray(capacity, 0), capacity_(capacity) {}

// hash functions
size_t BloomFilter::hash1(const std::string &item) const
{
    // primary hash function using std::hash
    return std::hash<std::string>{}(item);
}

size_t BloomFilter::hash2(const std::string &item) const
{
    // secondary hash function with salt to reduce collisions
    return std::hash<std::string>{}(item + "salt1");
}

size_t BloomFilter::hash3(const std::string &item) const
{
    // third hash function with different salt
    return std::hash<std::string>{}(item + "salt2");
}

// insertion
void BloomFilter::insert(const std::string &item)
{
    // set bits at positions determined by hash functions
    bitArray[hash1(item) % capacity_] = 1;
    bitArray[hash2(item) % capacity_] = 1;
    bitArray[hash3(item) % capacity_] = 1;
}

// query
bool BloomFilter::contains(const std::string &item) const
{
    // check if all corresponding bits are set
    return bitArray[hash1(item) % capacity_] &&
           bitArray[hash2(item) % capacity_] &&
           bitArray[hash3(item) % capacity_];
}