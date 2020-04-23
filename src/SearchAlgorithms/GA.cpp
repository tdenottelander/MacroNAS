//
//  GA.cpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "GA.hpp"

using namespace std;

extern int populationInitializationMode;

/* ------------------------ Genetic Algorithm ------------------------ */

GA::GA(FitnessFunction * fitfunc) :
    terminated(false),
    initialized(false),
    fitFunc_ptr(fitfunc),
    roundsCount(0)
{}

void GA::initialize(){
    switch (populationInitializationMode){
        case 0: initializeTrueRandomPopulation(); break;
        case 1: initializeNASPopulation(); break;
    }
    terminated = false;
    converged = false;
    roundsCount = 0;
    noAdditionToElitistArchiveCount = 0;
    initialized = true;
}

void GA::run(){
    while(!isConverged()){
        round();
    }
}

void GA::round(){
    roundPOVariationSelection();
    roundsCount++;
}

void GA::roundReplacementVariationSelection(){
    population = variation_ptr->variate(population);
    evaluateAll(population);
    population = selection_ptr->select(population, population.size());
}

void GA::roundPOVariationSelection(){
    vector<Individual> offspring = variation_ptr->variate(population);
    evaluateAll(offspring);
    
    vector<Individual> PO;
    PO.reserve(population.size() * 2);
    PO.insert(PO.begin(), population.begin(), population.end());
    PO.insert(PO.end(), offspring.begin(), offspring.end());

    population = selection_ptr->select(PO, population.size());
}

void GA::evaluateAll(vector<Individual> &population){
    for(Individual &ind: population){
        if(fitFunc_ptr->optimumFound || fitFunc_ptr->maxEvaluationsExceeded() || fitFunc_ptr->maxUniqueEvaluationsExceeded() || fitFunc_ptr->maxNetworkUniqueEvaluationsExceeded()){
            return;
        }
        fitFunc_ptr->evaluate(ind);
    }
}

void GA::evaluateAll(){
    evaluateAll(population);
}

vector<float> GA::getAvgFitness(){
    vector<float> result (population[0].fitness.size(), 0);
    for(Individual ind: population){
        for (int obj = 0; obj < ind.fitness.size(); obj++){
            result[obj] += ind.fitness[obj];
        }
    }
    for (int obj = 0; obj < population[0].fitness.size(); obj++){
        result[obj] = result[obj] / population.size();
    }
    return result;
}

bool GA::isConverged(){
    if(converged || isOptimal() || (!isDiverse() && !isLocalSearchAlgorithm && !isRandomSearchAlgorithm)){
        converged = true;
        terminated = true;
        return true;
    }
    return false;
}

bool GA::isDiverse(){
    for (int i = 1; i < population.size(); i++){
        if (!population[i].equals(population[0])){
            return true;
        }
    }
    return false;
}

bool GA::isOptimal(){
    return fitFunc_ptr->optimumFound;
}

int GA::getTotalAmountOfEvaluations(){
    return fitFunc_ptr->totalEvaluations;
}

void GA::initializeTrueRandomPopulation(){
    population = vector<Individual>();
    population.reserve(populationSize);
    vector<int> alphabet = fitFunc_ptr->problemType->alphabet;
    for(int i = 0; i < populationSize; i++){
        Individual ind (fitFunc_ptr->totalProblemLength, fitFunc_ptr->numObjectives);
        ind.initialize(alphabet);
        population.push_back(ind);
    }
}

void GA::initializeNASPopulation(){
    initializeTrueRandomPopulation();
    for (int i = 0; i < population[0].genotype.size(); i++){
        population[0].genotype[i] = 0;
    }
}

string GA::toString() {
    return populationToString(population);
}

string GA::populationToString(vector<Individual> &population){
    string result;
    string fitfuncid = fitFunc_ptr->id();
    bool trapfunc = fitfuncid.at(0) == 'T';
    for (unsigned long i = 0; i < population.size(); i++){
        result += to_string(i);
        result += ". ";
        if(trapfunc){
            result += population[i].toStringBlocks(stoi(fitfuncid.substr(1, fitfuncid.length())));
        } else {
            result += population[i].toString();
        }
        if(i != population.size() - 1){
            result += "\n";
        }
    }
    return result;
}

void GA::print(){
    cout << toString() << endl;
}

void GA::setPopulationSize(int n){
    populationSize = n;
}

void GA::setFitnessFunction (FitnessFunction * fitfunc){
    fitFunc_ptr = fitfunc;
}

void GA::setSelection (Selection * sel){
    selection_ptr = sel;
}

void GA::setVariation (Variation * var){
    variation_ptr = var;
}

void GA::setPointers(FitnessFunction * fitfunc, Selection * sel, Variation * var){
    setFitnessFunction(fitfunc);
    setSelection(sel);
    setVariation(var);
}

GA* GA::clone() const {
    return new GA(static_cast<const GA&>(*this));
}

string GA::id(){
    return "Base GA";
}
