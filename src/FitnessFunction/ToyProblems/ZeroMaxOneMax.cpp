//
//  ZeroMaxOneMax.cpp
//  GA
//
//  Created by Tom den Ottelander on 04/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "ZeroMaxOneMax.hpp"

using namespace std;

ZeroMaxOneMax::ZeroMaxOneMax (int problemSize) : FitnessFunction(new BinaryProblemType()) {
    setLength(problemSize);
    setNumObjectives(2);
    setOptimum(vector<float>(2, problemSize));
}

vector<float> ZeroMaxOneMax::evaluate(Individual &ind) {
    vector<float> result (numObjectives, 0);
    
    for (int i = 0; i < totalProblemLength; i++){
        if (ind.genotype[i] == 0){
            result[0] += 1;
        } else if (ind.genotype[i] == 1){
            result[1] += 1;
        }
    }
    ind.fitness = result;
    
    evaluationProcedure(ind);
    
    return result;
}

FitnessFunction* ZeroMaxOneMax::clone() const {
    return new ZeroMaxOneMax(static_cast<const ZeroMaxOneMax&>(*this));
}

void ZeroMaxOneMax::setOptimum(vector<float> opt){
    optimum = opt;
    optimalParetoFrontSize = totalProblemLength + 1;
    for (int i = 0; i < optimalParetoFrontSize; i++){
        trueParetoFront.push_back({(float)i, (float)totalProblemLength - i});
    }
}

bool ZeroMaxOneMax::entireParetoFrontFound() {
    if (elitistArchive.size() == optimalParetoFrontSize){
        for (Individual &ind : elitistArchive){
            if((ind.fitness[0] + ind.fitness[1]) != totalProblemLength){
                return false;
            }
        }
        return true;
    }
    return false;
}

string ZeroMaxOneMax::id() {
    return ("zmom");
}
