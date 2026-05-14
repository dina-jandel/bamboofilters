#pragma once

#include <vector>
#include <string>
#include <functional>
#include <cstdint>

class BloomFilter
{
public:
    BloomFilter(size_t capacity = 1000000);

    void insert(const std::string &item);
    bool contains(const std::string &item) const;

private:
    std::vector<uint8_t> bitArray;
    size_t capacity_;

    size_t hash1(const std::string &item) const;
    size_t hash2(const std::string &item) const;
    size_t hash3(const std::string &item) const;
};