#include "BambooFilter.hpp"

BambooFilter::BambooFilter(size_t size, size_t hashCount)
    : size(size),
      hashCount(hashCount),
      table(size, false)
{
}

size_t BambooFilter::hash(const std::string &s, size_t seed) const
{
    size_t h = seed;

    for (char c : s)
    {
        h = h * 31 + static_cast<size_t>(c);
    }

    return h % size;
}

void BambooFilter::insert(const std::string &item)
{
    for (size_t i = 0; i < hashCount; i++)
    {
        size_t index = hash(item, i + 1);

        table[index] = true;
    }
}

bool BambooFilter::contains(const std::string &item) const
{
    for (size_t i = 0; i < hashCount; i++)
    {
        size_t index = hash(item, i + 1);

        if (!table[index])
        {
            return false;
        }
    }

    return true;
}