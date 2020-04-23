//
//  UniqueSolutions.cpp
//  GA
//
//  Created by Tom den Ottelander on 18/12/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "SolutionLibrary.hpp"

using namespace std;
using namespace nlohmann;

// Constructor
SolutionLibrary::SolutionLibrary(Type type) : type(type), library(unordered_map<string, vector<float>>()){
}

// Inserts a hashed genotype into the unorderd set, regardless of already being in it
void SolutionLibrary::put(vector<int> &genotype, vector<float> &fitness){
    library[hash(genotype)] = fitness;
}

// Returns true when the unordered set already contains the hash if this genotype
bool SolutionLibrary::contains(vector<int> &genotype){
    return (library.find(hash(genotype)) != library.end());
}

vector<float> SolutionLibrary::get(vector<int> &genotype){
    return library[hash(genotype)];
}

void SolutionLibrary::clear(){
    library.clear();
}

string SolutionLibrary::hash(vector<int> &genotype){
    switch (type) {
        case Type::DEFAULT:
            return hash_DEFAULT(genotype);
        case Type::LARGE_SCALE:
            return hash_LARGESCALE(genotype);
        case Type::BENCHMARK:
            return hash_BENCHMARK(genotype);
    }
    return "";
}

string SolutionLibrary::hash_BENCHMARK(vector<int> &genotype){
    string res = "";
    for (int i = 0; i < genotype.size(); i++){
        if (i != 0 && (i % 4 == 0)){
            res += ".";
        }
        if (genotype[i] != 0){
            res += to_string(genotype[i]);
        }
    }
    return res;
}

string SolutionLibrary::hash_LARGESCALE(vector<int> &genotype){
    string res = "";
    for (int i = 0; i < genotype.size(); i++){
        if (i != 0 && ((i % 5 == 0) || (i % 5 == 4))){
            res += ".";
        }
        if ((i % 5 == 4) || genotype[i] != 0){
            res += to_string(genotype[i]);
        }
    }
    return res;
}

string SolutionLibrary::hash_DEFAULT(vector<int> &genotype){
    string res = "";
    for (int i = 0; i < genotype.size(); i++){
        res += to_string(genotype[i]);
    }
    return res;
}


SolutionCounter::SolutionCounter (int alphabetSize, int problemSize) : alphabetSize(alphabetSize), problemSize(problemSize) {}

void SolutionCounter::put(vector<int> &genotype){
    long hash = HashingFunctions::hash(genotype, alphabetSize);
    int count = 1;
    if(contains(genotype)){
        count += counterMap.at(hash);
        counterMap.erase(hash);
    }
    pair<long, int> insertion (hash, count);
    counterMap.insert(insertion);
}

bool SolutionCounter::contains(vector<int> &genotype){
    return (counterMap.find(HashingFunctions::hash(genotype, alphabetSize)) != counterMap.end());
}

long SolutionCounter::get(vector<int> &genotype){
    if(contains(genotype)){
        return counterMap.at(HashingFunctions::hash(genotype, alphabetSize));
    } else {
        return 0;
    }
}

json SolutionCounter::toJson (bool asHash){
    json result;
    for (auto it = counterMap.begin(); it != counterMap.end(); ++it ){
        if(asHash)
            result[to_string(it->first)] = it->second;
        else {
            vector<int> genotype = HashingFunctions::decode(it->first, problemSize , alphabetSize);
            result[Utility::genotypeToString(genotype)] = it->second;
        }
    }
    return result;
}


// Hashes an individual's genotype by enumeration like so:
// 0000000 - 0
// 0000001 - 1
// 0000002 - 2
// 0000010 - 3
// 0000011 - 4
// ...etc
long HashingFunctions::hash(vector<int> &genotype, int alphabetSize){
    long result = 0;
    int n = genotype.size();
    //    for (int i = 0; i < n; i++){
    //        result += genotype[i] * pow(alphabetSize, n-i-1);
    //    }
    
    for (int i = 0; i < n; i++){
        result += genotype[n-i-1] * pow(alphabetSize, i);
    }
    return result;
}

vector<int> HashingFunctions::decode(long hash, int problemSize, int alphabetSize){
    vector<int> genotype (problemSize, 0);
    for(int i = 0; i < problemSize; i++){
        long sub = pow(alphabetSize, problemSize - i - 1);
        int layer = floor(hash / sub);
        genotype[i] = layer;
        hash = hash - layer * sub;
    }
    
    return genotype;
}


string HashingFunctions::toString(vector<int> &genotype, string type){
    string res = "";
    for (int i = 0; i < genotype.size(); i++){
        if (type == "default"){
            res += to_string(genotype[i]);
        } else if (type == "largescale"){
            if (i != 0 && (i % 5 == 0) || (i % 5) == 4){
                res += ".";
            }
            if ((i % 5 == 4) || genotype[i] != 0){
                res += to_string(genotype[i]);
            }
        }
    }
        
    return res;
}

vector<int> HashingFunctions::toGenotype(string str){
    vector<int> res (str.length(), 0);
    for (int i = 0; i < str.length(); i++){
        res[i] = (int)str.at(i) - 48; // Subtract 48 for ASCII characters
    }
    return res;
}
