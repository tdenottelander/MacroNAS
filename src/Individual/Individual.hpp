//
//  Individual.hpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef Individual_hpp
#define Individual_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "Utility.hpp"

class Individual {
public:
    std::vector<int> genotype;
    std::vector<float> fitness;
    int counterNotChanged;
    
    // MO-fields
    int dominationCount;
    std::vector<Individual*> dominationList;
    int front;
    float crowdingDistance;
    
    Individual();
    Individual(int length, int objectives);
    void initialize(std::vector<int> &alphabet);
    Individual copy();
    std::string toString();
    std::string toStringBlocks(int blocksize);
    std::string toStringFitness();
    bool equals(Individual const& ind);
    bool genotypeEquals(std::vector<int> &genotype);
    bool fitnessEquals(Individual &ind, float margin = 0.000001);
    bool dominates(Individual &indOther);
    void clearMOinformation();
    
    static std::string toString(std::vector<int> &genotype);
    static int hammingDistance(std::vector<int> &g1, std::vector<int> &g2);
    static int editDistance(std::vector<int> &g1, std::vector<int> &g2);
    static int editDistance(std::vector<int> &g1, std::vector<int> &g2, int i, int j);
    static std::vector<int> removeIdentities(std::vector<int> &genotype, int identityLayerIndex);
    static bool genotypeEquals(std::vector<int> &g1, std::vector<int> &g2);
    static bool dominates(Individual &ind1, Individual &ind2);
};

#endif /* Individual_hpp */


