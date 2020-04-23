//
//  LocalSearchBase.cpp
//  GA
//
//  Created by Tom den Ottelander on 07/01/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "LocalSearchBase.hpp"

using namespace std;

LocalSearchBase::LocalSearchBase (FitnessFunction * fitFunc, Utility::Order localSearchOrder, float stochasticity, bool loop) : GA(fitFunc),  stochasticity(stochasticity), loop(loop), localSearchOrder(localSearchOrder){
    isLocalSearchAlgorithm = true;
}

void LocalSearchBase::round(){
    
    Individual ind = population[0];
    
    ind.initialize(fitFunc_ptr->problemType->alphabet);
    
    doLocalSearch(ind, fitFunc_ptr);
    
    roundsCount++;
}

void LocalSearchBase::doLocalSearch(Individual &ind, FitnessFunction * fitfunc, int maxRandomFlips){
    bool converged = false;
    int randomFlips = 0;
    vector<int> alphabet = fitfunc->problemType->alphabet;
    
    do {
        converged = true;
        
        int probSize = fitfunc->totalProblemLength;
        vector<int> order = Utility::getOrderedArray(probSize, localSearchOrder);
        
        for(int index : order){
            
            int originalLayer = ind.genotype[index];
            int newLayer = originalLayer;
            
            if((randomFlips < maxRandomFlips || maxRandomFlips == -1) && Utility::getRand() < stochasticity){
                newLayer = Utility::getRand(0, alphabet.size());
                ind.genotype[index] = newLayer;
                fitfunc->evaluate(ind);
                randomFlips += 1;
            } else {
            
                float bestFitness = ind.fitness[0];
                
                for(int layertype : alphabet){
                    if(layertype != originalLayer) {
                        ind.genotype[index] = layertype;
                        vector<float> fitness = fitfunc->evaluate(ind);
                        if(fitness[0] > bestFitness){
                            newLayer = layertype;
                            bestFitness = fitness[0];
                        }
                    }
                }
                
                ind.genotype[index] = newLayer;
                ind.fitness[0] = bestFitness;
            }
            
            if (newLayer != originalLayer){
                converged = false;
            }
        }
    } while(!converged && loop);
}
