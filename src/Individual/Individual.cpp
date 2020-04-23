//
//  Individual.cpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "Individual.hpp"

using namespace std;
using namespace Utility;


/* ------------------------ Individual ------------------------ */

Individual::Individual() : Individual::Individual(0, 1) {}

Individual::Individual(int length, int objectives) :
    fitness(objectives, -1),
    counterNotChanged(0),
    dominationCount(0),
    front(-1),
    crowdingDistance(0.0)
{
    genotype = vector<int>(length, 0);
}

void Individual::initialize(vector<int> &alphabet){
    int n = alphabet.size();
    for(int i = 0; i < genotype.size(); i++){
        genotype[i] = alphabet[floor(Utility::getRand() * n)];
    }
}

Individual Individual::copy(){
    int l = genotype.size();
    int o = fitness.size();
    Individual ind(l, o);
    for(int i = 0; i < l; i++){
        ind.genotype[i] = genotype[i];
    }
    for(int i = 0; i < o; i++){
        ind.fitness[i] = fitness[i];
    }
    ind.crowdingDistance = crowdingDistance;
    ind.front = front;
    ind.counterNotChanged = counterNotChanged;
    ind.dominationCount = dominationCount;
    return ind;
}

string Individual::toString(){
    string result = toString(genotype);
    result += "  " + toStringFitness();
//    result += "  CD: " + to_string(crowdingDistance);
    return result;
}

string Individual::toStringBlocks(int blocksize){
    string result = "[";
    for (unsigned long i = 0; i < genotype.size(); i++){
        result += to_string(genotype[i]);
        if(i != (genotype.size() - 1)){
            result += " ";
            if((i+1) % blocksize == 0){
                result += "| ";
            }
        }
    }
    result += "]  ";
    result += toStringFitness();
    return result;
}

string Individual::toStringFitness(){
    string result = "F: ";
    for (int i = 0; i < fitness.size(); i++){
        result += to_string(fitness[i]);
        if (i < fitness.size() - 1)
            result += ", ";
    }
    return result;
}

bool Individual::equals(const Individual &ind) {
    for (int i = 0; i < fitness.size(); i++){
        if (fitness[i] != ind.fitness[i]){
            return false;
        }
    }
    for (unsigned long i = 0; i < genotype.size(); i++){
        if(genotype[i] != ind.genotype[i]){
            return false;
        }
    }
    return true;
}

bool Individual::genotypeEquals(vector<int> &g){
    return genotypeEquals(g, genotype);
}

bool Individual::fitnessEquals(Individual &ind, float margin){
    for (int i = 0; i < fitness.size(); i++){
        if (abs(fitness[i] - ind.fitness[i]) > margin){
            return false;
        }
    }
    return true;
}

bool Individual::dominates(Individual &indOther){
    return dominates(*this, indOther);
}

void Individual::clearMOinformation(){
    dominationCount = 0;
    dominationList.clear();
    front = -1;
    crowdingDistance = 0.0;
}

string Individual::toString(vector<int> &genotype){
    string result = "[";
    for (unsigned long i = 0; i < genotype.size(); i++){
        result += to_string(genotype[i]);
        if(i != (genotype.size() - 1)){
            result += " ";
        }
    }
    result += "]";
    return result;
}

int Individual::hammingDistance(vector<int> &g1, vector<int> &g2){
    if (g1.size() != g2.size()){
        cout << "Error: Cannot compare hamming distance between two genotypes of different length" << endl;
        return -1;
    }
    int distance = 0;
    for (int i = 0; i < g1.size(); i++){
        if (g1[i] != g2[i])
            distance++;
    }
    return distance;
}

int Individual::editDistance(vector<int> &g1, vector<int> &g2){
    return editDistance(g1, g2, g1.size(), g2.size());
}

int Individual::editDistance(vector<int> &g1, vector<int> &g2, int i, int j){
    if(i == 0 || j == 0){
        return max(i, j);
    } else {
        int minCase1 = editDistance(g1, g2, i-1, j) + 1;
        int minCase2 = editDistance(g1, g2, i, j-1) + 1;
        int indicator = 1;
        if(g1[i-1] == g2[j-1])
            indicator = 0;
        int minCase3 = editDistance(g1, g2, i-1, j-1) + indicator;
        return min(minCase1, min(minCase2, minCase3));
    }
}

vector<int> Individual::removeIdentities(vector<int> &genotype, int identityLayerIndex){
    vector<int> newGenotype (genotype.size(), 0);
    int j = 0;
    for (int i = 0; i < genotype.size(); i++){
        if (genotype[i] != identityLayerIndex){
            newGenotype[j] = genotype[i];
            j++;
        }
    }
    vector<int> newVec (newGenotype.begin(), newGenotype.begin() + j);
    return newVec;
}

bool Individual::genotypeEquals(vector<int> &g1, vector<int> &g2){
    if(g1.size() != g2.size()){
        return false;
    }
    for (int i = 0; i < g1.size(); i++){
        if (g1[i] != g2[i]){
            return false;
        }
    }
    return true;
}

// Returns true if [ind1] dominates [ind2]
// Notion of domination used: When all objectives are equal or better and at least one objective is strictly better.
bool Individual::dominates(Individual &ind1, Individual &ind2){
    bool domination = false;
    for (int obj = 0; obj < ind1.fitness.size(); obj++){
        if (ind1.fitness[obj] > ind2.fitness[obj]){
            domination = true;
        } else if (ind1.fitness[obj] < ind2.fitness[obj]){
            return false;
        }
    }
    return domination;
}
