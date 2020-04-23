//
//  RS.cpp
//  GA
//
//  Created by Tom den Ottelander on 26/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "RS.hpp"

using namespace std;

RS::RS (FitnessFunction * fitFunc) : GA(fitFunc){
    preventIMS = true;
    isRandomSearchAlgorithm = true;
}

void RS::round(){
    Individual &ind = population[0];
    
    ind.initialize(fitFunc_ptr->problemType->alphabet);
    
    fitFunc_ptr->evaluate(ind);
    fitFunc_ptr->updateElitistArchive(&ind);
    
    roundsCount++;
}

GA* RS::clone() const {
    return new RS(static_cast<const RS&>(*this));
}

string RS::id() {
    return "RS";
}
