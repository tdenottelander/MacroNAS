//
//  TrapInverseTrap.cpp
//  GA
//
//  Created by Tom den Ottelander on 06/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "TrapInverseTrap.hpp"

using namespace std;

TrapInverseTrap::TrapInverseTrap(int length) : FitnessFunction(new BinaryProblemType()) {
    if (length % blocksize != 0){
        cout << "ERROR: TrapInverseTrap length is not a multiple of blocksize " << blocksize << endl;
    }
    blocks = length / blocksize;
    totalProblemLength = length;
    optimalParetoFrontSize = blocks + 1;
    numObjectives = 2;
    for (int i = 0; i < optimalParetoFrontSize; i++){
        trueParetoFront.push_back({(float)totalProblemLength - i, (float)totalProblemLength - (optimalParetoFrontSize - i - 1)});
    }
}

// Returns the fitness of an individual
vector<float> TrapInverseTrap::evaluate(Individual &ind) {
    vector<float> result (numObjectives, 0.0);
    for (int obj = 0; obj < numObjectives; obj++){
        for (int i = 0; i < blocks; i++) {
            result[obj] += subfunc(ind, i * blocksize, i * blocksize + blocksize, obj);
        }
    }

    ind.fitness = result;
    
    evaluationProcedure(ind);
    
    return result;
}

// Returns the fitness of a block of genes.
float TrapInverseTrap::subfunc(Individual &ind, int startIdx, int endIdx, int objective) {
    int u = 0;
    for(int i = startIdx; i < endIdx; i++){
        if (ind.genotype[i] == objective){
            u += 1;
        }
    }

    if (u == blocksize){
        return blocksize;
    } else {
        return (blocksize - 1.0) - u;
    }
}

void TrapInverseTrap::display() {
    cout << "Trap(" << blocksize << ") fitness function" << endl;
}

string TrapInverseTrap::id() {
    return ("T" + to_string(blocksize));
}

FitnessFunction* TrapInverseTrap::clone() const {
    return new TrapInverseTrap(static_cast<const TrapInverseTrap&>(*this));
}

bool TrapInverseTrap::entireParetoFrontFound() {
    if (elitistArchive.size() == optimalParetoFrontSize){
        for (Individual &ind : elitistArchive){
            // A pareto optimal solution consists of full blocks of 1's and/or 0's.
            // So for two blocks, the optimal solutions are:
            //  11111|11111,   00000|11111,   11111|00000,   00000|00000
            // F:   8/10           9/9            9/9           10/8
            // So summed optimal fitness = blocks * blocksize * 2 - blocks
            if((ind.fitness[0] + ind.fitness[1]) != (blocks * blocksize * 2 - blocks)){
                return false;
            }
        }
        return true;
    }
    return false;
}
