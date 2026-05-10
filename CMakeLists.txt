cmake_minimum_required(VERSION 3.10)

project(BambooFilters)

set(CMAKE_CXX_STANDARD 17)

add_executable(bamboo_project
    src/main.cpp
    src/filter/BambooFilter.cpp
    src/bio/DNAGenerator.cpp
    src/bio/KMerGenerator.cpp
    src/utils/Benchmark.cpp
    src/utils/FASTAParser.cpp
)