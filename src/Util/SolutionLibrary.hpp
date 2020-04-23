//
//  UniqueSolutions.hpp
//  GA
//
//  Created by Tom den Ottelander on 18/12/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef UniqueSolutions_hpp
#define UniqueSolutions_hpp

#include <stdio.h>
#include <unordered_set>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "Utility.hpp"

class SolutionLibrary{
public:
    enum class Type {DEFAULT, LARGE_SCALE, BENCHMARK};

    SolutionLibrary(Type = Type::DEFAULT);
    Type type;
    std::unordered_map<std::string, std::vector<float>> library;
    void put(std::vector<int> &genotype, std::vector<float> &fitness);
    bool contains(std::vector<int> &genotype);
    std::vector<float> get(std::vector<int> &genotype);
    void clear();
    std::string hash(std::vector<int> &genotype);
    std::string hash_BENCHMARK(std::vector<int> &genotype);
    std::string hash_LARGESCALE(std::vector<int> &genotype);
    std::string hash_DEFAULT(std::vector<int> &genotype);
};

class SolutionCounter {
public:
    SolutionCounter(int alphabetSize, int problemSize);
    int problemSize;
    int alphabetSize;
    std::unordered_map<int, int> counterMap;
    void put(std::vector<int> &genotype);
    bool contains(std::vector<int> &genotype);
    long get(std::vector<int> &genotype);
    nlohmann::json toJson (bool asHash);
};

namespace HashingFunctions {
    long hash(std::vector<int> &genotype, int alphabetSize);
    std::vector<int> decode(long hash, int problemSize, int alphabetSize);
    std::string toString(std::vector<int> &genotype, std::string type="default");
    std::vector<int> toGenotype(std::string str);
};

#endif /* UniqueSolutions_hpp */
