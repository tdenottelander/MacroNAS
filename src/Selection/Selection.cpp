//
//  Selection.cpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "Selection.hpp"

using namespace std;
using namespace Utility;


/* ------------------------ Base Selection ------------------------ */

Selection::Selection(){}

void Selection::display(){
    cout << "Base selection" << endl;
}

string Selection::id(){
    return "base";
}


/* ------------------------ Proportionate Selection ------------------------ */

ProportionateSelection::ProportionateSelection(){}

vector<Individual> ProportionateSelection::select(vector<Individual> &population, int outputSize) {
    vector<Individual> selection;
    selection.reserve(outputSize);
    vector<float> proportions = getProportions(population);
    for(int i = 0; i < outputSize; i++){
        float randomValue = Utility::getRand();
        for (unsigned long j = 0; j < proportions.size(); j++){
            if(randomValue < proportions[j]){
                selection.push_back(population[j]);
                break;
            }
        }
    }
    return selection;
}

void ProportionateSelection::display() {
    cout << "Proportionate selection" << endl;
}

string ProportionateSelection::id(){
    return "prop";
}

vector<float> ProportionateSelection::getProportions(vector<Individual> &population){
    int n = (int)population.size();
    vector<float> proportions (n);
    int cumulativeFitness = 0;
    
    for(Individual ind: population){
        cumulativeFitness += ind.fitness[0];
    }
    
    for(int i = 0; i < n; i++){
        if(i == 0) {
            proportions[i] = (float)population[i].fitness[0] / cumulativeFitness;
        } else {
            proportions[i] = proportions[i-1] + (float)population[i].fitness[0] / cumulativeFitness;
        }
    }
    
    return proportions;
}


/* ------------------------ Tournament Selection ------------------------ */

TournamentSelection::TournamentSelection(int tournamentSize) : tournamentSize(tournamentSize) {}

vector<Individual> TournamentSelection::select(vector<Individual> &population, int outputSize){
    int n = (int)population.size();
    vector<Individual> newPopulation;
    newPopulation.reserve(outputSize);
    
    int rounds = tournamentSize * outputSize / n;
    
    for (int round = 0; round < rounds; round++){
        vector<int> randomIndices = Utility::getRandomlyPermutedArrayV2(n);
        for (int i = 0; i < (n/tournamentSize); i++){
            vector<Individual*> selectedIndividuals;
            selectedIndividuals.reserve(tournamentSize);
//            cout << "Tournament with " << tournamentSize << " individuals:" << endl;
            for (int j = 0; j < tournamentSize; j++){
                int index = randomIndices[i * tournamentSize + j];
                selectedIndividuals.push_back(&population[index]);
//                cout << index << ": " << population[index].toString() << endl;
            }
            
            Individual* winner = tournament(selectedIndividuals);
            
            newPopulation.push_back(winner->copy());
        }
    }
    
    return newPopulation;
}

Individual* TournamentSelection::tournament(vector<Individual *> selectedIndividuals){
    int n = selectedIndividuals.size();
    int bestIndIdx = 0;
    int bestFitness = selectedIndividuals[0]->fitness[0];
    for(int i = 1; i < n; i++){
        if (selectedIndividuals[i]->fitness[0] > bestFitness){
            bestIndIdx = i;
            bestFitness = selectedIndividuals[i]->fitness[0];
        }
    }
    return selectedIndividuals[bestIndIdx];
}

void TournamentSelection::display() {
    cout << "Tournament selection" << endl;
}

string TournamentSelection::id(){
    return ("tour" + to_string(tournamentSize));
}
