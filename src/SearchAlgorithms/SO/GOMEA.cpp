//
//  GOMEA.cpp
//  GA
//
//  Created by Tom den Ottelander on 28/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "GOMEA.hpp"

using namespace std;

GOMEA::GOMEA(FitnessFunction * fitfunc, FOS *fos, bool forcedImprovement = false) : GA(fitfunc), fos(fos), gom_variation(*new GOM_Variation(forcedImprovement)) {
}

void GOMEA::initialize() {
    GA::initialize();
    gom_variation.fitfunc = fitFunc_ptr;
    gom_variation.setFOSObject(fos);
}

void GOMEA::round() {
    //TODO: Make the evaluations stop when the max number of evaluations is reached
    vector<Individual> offspring = gom_variation.variate(population);
    
    population = offspring;
    
    roundsCount++;
}

GA* GOMEA::clone() const {
    return new GOMEA(static_cast<const GOMEA&>(*this));
}

string GOMEA::id(){
    return "GOM_" + fos->id(); // + "_FI=" + (gom_variation.forcedImprovement ? "t" : "f");
}
