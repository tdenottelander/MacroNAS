//
//  LS.cpp
//  GA
//
//  Created by Tom den Ottelander on 07/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "LS.hpp"

using namespace std;

// 0 = True Random, 1 = NAS (first one is all identity individual), 2 = Solvable
extern int populationInitializationMode;
extern bool saveLSArchiveResults;
extern nlohmann::json JSON_MO_info;
extern nlohmann::json JSON_LS_Results;

LS::LS (FitnessFunction * fitfunc) : LS::LS (fitfunc, Utility::Order::RANDOM, false, NewScalarization::RANDOM) {};

LS::LS (FitnessFunction * fitfunc, Utility::Order order, bool loop, NewScalarization newScalarization) : GA(fitfunc), LS_order(order), loop(loop), newScalarization(newScalarization) {
    isLocalSearchAlgorithm = true;
    cout << "traversal order: " << Utility::orderToID(order) << endl;
    cout << "loop: " << loop << endl;
    cout << "NewScalarization: " << NewScalarizationToString(newScalarization) << endl;

}

void LS::round(){
    if(roundsCount == 0){
        // If we do not search for random scalarizations, first search in both extreme directions.
        if (newScalarization != NewScalarization::RANDOM){
            scalarizationTargets.push(0.0f); // In the direction of efficient network (MMACS)
            scalarizationTargets.push(1.0f); // In the direction of good predicting network (validation
        }

        // Do this if the all-identity solution is added beforehand:
        if (populationInitializationMode == 1){
            if (!scalarizationTargets.empty()) scalarizationTargets.pop();
            updateLSArchive({0.0f, 1.0f}, population[0].fitness);
        }
    }

    // Getting the next scalarization value
    if (scalarizationTargets.empty()){
        scalarizationTargets.push(getNewScalarizationTarget());
    }
    float scalarization = scalarizationTargets.front();
    scalarizationTargets.pop();

    // Initialize individual and evaluate
    Individual &ind = population[0];
    ind.initialize(fitFunc_ptr->problemType->alphabet);
    fitFunc_ptr->evaluate(ind);

    // Perform local search
    performLocalSearch(ind, vector<float>{scalarization, 1.0f - scalarization});

    // Add scalarization to the archive
    if (saveLSArchiveResults){
        pair<float,float> sc {scalarization, 1.0f - scalarization};
        updateLSArchive(sc, ind.fitness);
        JSON_LS_Results["LS_converged_solutions"] = LS_archive.size();
        JSON_LS_Results["LS_archive"] = LS_archive;
    }

    roundsCount++;
}

void LS::performLocalSearch(Individual &ind, vector<float> scalarization){
    bool changed = true;
    do {
        changed = false;
        vector<int> randIndexArray = Utility::getOrderedArray(ind.genotype.size(), LS_order);
        for (int i : randIndexArray){
            Individual copiedIndividual = ind.copy();
            
            // It does not matter in which order we loop over the available layers, because we search exhaustively and two different layers will have epsilon chance to result in the same objective values.
            for (int bit : fitFunc_ptr->problemType->alphabet){
                copiedIndividual.genotype[i] = bit;
                
                if(fitFunc_ptr->isDone()){
                    return;
                }
                
                fitFunc_ptr->evaluate(copiedIndividual);
                if (dominates(copiedIndividual, ind, scalarization)){
                    ind.genotype[i] = bit;
                    ind.fitness = copiedIndividual.fitness;
                    changed = true;
                }
            }
        }
    } while (changed && loop);
}

// Returns true if [indThis] dominates [indOther]
// Domination is based on the difference between scalarized fitness of the individuals.
bool LS::dominates(Individual &indThis, Individual &indOther, vector<float> scalarization){
    float scalarizedFitnessThis =scalarizeFitness(indThis, scalarization);
    float scalarizedFitnessOther = scalarizeFitness(indOther, scalarization);
    return scalarizedFitnessThis > scalarizedFitnessOther;
}

// Scalarizes an individual's fitness using the scalarization values.
float LS::scalarizeFitness(Individual &ind, vector<float> scalarization){
    float scalarizedFitness = 0.0;
    for (int obj = 0; obj < ind.fitness.size(); obj++){
        scalarizedFitness += (scalarization[obj] * ind.fitness[obj]);
    }
    return scalarizedFitness;
}

float LS::getNewScalarizationTarget(){
    if (newScalarization == NewScalarization::RANDOM){
        return Utility::getRand();
    } else {
        // Sort the result array based on scalarization value
        sort(LS_archive.begin(), LS_archive.end(), [](pair<pair<float, float>, vector<float>> lhs, pair<pair<float, float>, vector<float>> rhs){
            return lhs.first.first < rhs.first.first;
        });
        float largestDistance = -1;
        vector<int> indices;
        for (int i = 0; i < LS_archive.size() - 1; i++){
            float dist = 0.0f;
            if (newScalarization == NewScalarization::OBJECTIVESPACE){
                dist = Utility::EuclideanDistanceSquared(LS_archive[i].second, LS_archive[i+1].second); // OBJECTIVE SPACE DISTANCE
            } else if (newScalarization == NewScalarization::SCALARIZATIONSPACE){
                dist = abs(LS_archive[i].first.first - LS_archive[i+1].first.first); // SCALARIZATION SPACE DISTANCE
            }
            
            if (dist == largestDistance){
                indices.push_back(i);
            }
            
            if (dist > largestDistance){
                indices.clear();
                indices.push_back(i);
                largestDistance = dist;
            }
        }
        
        int largestDistanceIdx = Utility::getRand(indices);
        float result = (LS_archive[largestDistanceIdx].first.first + LS_archive[largestDistanceIdx+1].first.first) / 2;
        return result;
    }
}

void LS::updateLSArchive(pair<float, float> scalarization, vector<float> objectiveValues){
    /* SETTING THE SCALARIZATION VALUE AS THE MEAN OF TWO SOLUTIONS WITH IDENTICAL OBJECTIVE VALUES
    for (int i = 0; i < old_archive.size(); i++){
        if(equalObjectiveValues(objectiveValues, old_archive[i].second)){
            float meanScalarization = (scalarization.first + old_archive[i].first.first) / 2;
            pair<float, float> newScalarization {meanScalarization, 1.0f - meanScalarization};
            old_archive.erase(old_archive.begin() + i);
            old_archive.push_back(pair<pair<float, float>, vector<float>> {newScalarization, objectiveValues});
            cout << "Found same fitness with other scalarization:         F: " << old_archive[i].second[0] << "|" << old_archive[i].second[1] << "   scalarization: " << old_archive[i].first.first << "|" << old_archive[i].first.second << ".... new scalarization: " << newScalarization.first << "|" << newScalarization.second << endl;
            return;
        }
    }
     */
    LS_archive.push_back(pair<pair<float, float>, vector<float>> {scalarization, objectiveValues});
}

bool LS::equalObjectiveValues(std::vector<float> &o1, std::vector<float> &o2){
    if (o1.size() != o2.size()){
        cout << "Objective values do not have the same size" << endl;
        return false;
    }
    for (int i = 0; i < o1.size(); i++){
        if (o1[i] != o2[i]){
            return false;
        }
    }
    return true;
}


GA* LS::clone() const {
    return new LS(static_cast<const LS&>(*this));
}

string LS::id(){
    string loopString = loop ? "loop" : "noloop";
    string dirString = "";
    switch (newScalarization) {
        case NewScalarization::RANDOM: dirString = "randdir"; break;
        case NewScalarization::SCALARIZATIONSPACE: dirString = "scalarizationdir"; break;
        case NewScalarization::OBJECTIVESPACE: dirString = "objectivedir"; break;
    }
    return ("LS-" + loopString + "-" + dirString);
}

string LS::NewScalarizationToString(NewScalarization newScalarization){
    switch (newScalarization) {
        case NewScalarization::RANDOM:
            return "RANDOM";
        case NewScalarization::SCALARIZATIONSPACE:
            return "SCALARIZATIONSPACE";
        case NewScalarization::OBJECTIVESPACE:
            return "OBJECTIVESPACE";
    }
    return "UNDEFINED";
}
