//
//  SimpleGA.cpp
//  GA
//
//  Created by Tom den Ottelander on 28/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "SimpleGA.hpp"
using namespace std;

SimpleGA::SimpleGA (FitnessFunction * fitFunc, Variation * var, Selection * sel) : GA(fitFunc) {
    variation_ptr = var;
    selection_ptr = sel;
}


void SimpleGA::roundPOVariationSelection(){
    vector<Individual> offspring = variation_ptr->variate(population);
    float chanceForMutation = 1.0f / fitFunc_ptr->totalProblemLength;
    Variation::mutate(offspring, chanceForMutation, fitFunc_ptr->problemType->alphabet);
    
    evaluateAll(offspring);
    
    vector<Individual> PO;
    PO.reserve(population.size() * 2);
    PO.insert(PO.begin(), population.begin(), population.end());
    PO.insert(PO.end(), offspring.begin(), offspring.end());
    
    population = selection_ptr->select(PO, population.size());
}

void SimpleGA::round(){
    roundPOVariationSelection();
    roundsCount++;
}

GA* SimpleGA::clone() const {
    return new SimpleGA(static_cast<const SimpleGA&>(*this));
}

string SimpleGA::id() {
    return "SimpleGA_s=" + selection_ptr->id() + "_v=" + variation_ptr->id();
}
