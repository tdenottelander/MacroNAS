//
//  LOTZ.cpp
//  GA
//
//  Created by Tom den Ottelander on 05/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "LOTZ.hpp"

using namespace std;

LOTZ::LOTZ (int problemSize) : FitnessFunction(new BinaryProblemType()) {
    setLength(problemSize);
    setNumObjectives(2);
    setOptimum(vector<float>(2, problemSize));
}

vector<float> LOTZ::evaluate(Individual &ind) {
    vector<float> result (numObjectives, 0);
    
    // First objective is to get the most leading ones
    for (int i = 0; i < totalProblemLength; i++){
        if (ind.genotype[i] == 1){
            result[0] = i + 1;
        } else {
            break;
        }
    }
    
    // Second objective is to get the most trailing zeros
    for (int i = totalProblemLength - 1; i >= 0; i--){
        if (ind.genotype[i] == 0){
            result[1] = totalProblemLength - i;
        } else {
            break;
        }
    }
    
    ind.fitness = result;
    
    evaluationProcedure(ind);
    
    return result;
}

FitnessFunction* LOTZ::clone() const {
    return new LOTZ(static_cast<const LOTZ&>(*this));
}

void LOTZ::setOptimum(vector<float> opt){
    optimum = opt;
    optimalParetoFrontSize = totalProblemLength + 1;
}

bool LOTZ::entireParetoFrontFound() {
    if (elitistArchive.size() == optimalParetoFrontSize){
        for (Individual &ind : elitistArchive){
            if ((ind.fitness[0] + ind.fitness[1]) != totalProblemLength){
                return false;
            }
        }
        return true;
    }
    return false;
}

string LOTZ::id() {
    return "lotz";
}
