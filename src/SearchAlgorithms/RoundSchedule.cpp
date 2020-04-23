//
//  RoundSchedule.cpp
//  GA
//
//  Created by Tom den Ottelander on 22/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "RoundSchedule.hpp"

using namespace std;
using nlohmann::json;
using Utility::millis;

extern bool printPopulationAfterRound;
extern bool printPopulationOnOptimum;
extern bool printProgressOnIntervals;
extern bool storeConvergence;
extern bool storeElitistArchive;

extern FitnessFunction * fitFunc;

nlohmann::json convergence;
extern json JSON_MO_info;
extern json JSON_SO_info;
extern json JSON_Run;
extern string path_JSON_Run;
extern bool saveLogFilesOnEveryUpdate;

RoundSchedule::RoundSchedule (int maxRounds, int maxPopSizeLevel, int maxSeconds, int maxEvaluations, int maxUniqueEvaluations, int maxNetworkUniqueEvaluations, int interleavedRoundInterval) :
    maxRounds(maxRounds),
    maxPopSizeLevel(maxPopSizeLevel),
    maxSeconds(maxSeconds),
    maxEvaluations(maxEvaluations),
    maxUniqueEvaluations(maxUniqueEvaluations),
    maxNetworkUniqueEvaluations(maxNetworkUniqueEvaluations),
    interval(interleavedRoundInterval)
{}

void RoundSchedule::initialize(GA *g, int problemSize, bool IMS, int nonIMSpopsize) {
    int beginPopSize = 8;
    
    // If IMS should not be used, or the algorithm doesn't require IMS (e.g. with LS or RS), then set the correct values.
    if(!IMS || g->isRandomSearchAlgorithm || g->isLocalSearchAlgorithm || g->preventIMS){
        maxPopSizeLevel = 1;
        beginPopSize = nonIMSpopsize;
        if(g->isLocalSearchAlgorithm || g->isRandomSearchAlgorithm){
            beginPopSize = 1;
        }
    }
    gaList.reserve(maxPopSizeLevel);

    JSON_Run["successfulGAPopulation"] = -1;
    JSON_Run["successfulGARoundCount"] = -1;
    JSON_Run["popsizereached"] = -1;
    JSON_Run["success"] = false;
    JSON_Run["stoppingCondition"] = "-1";
    
    whichShouldRun = vector<int>(maxPopSizeLevel, 0);
    for(int i = 0; i < maxPopSizeLevel; i++){
        int popSize = beginPopSize * pow(2, i);
        GA* newGA = g->clone();
        newGA->setPopulationSize(popSize);
        gaList.push_back(newGA);
    }
    whichShouldRun[0] = 1;
    
    convergence.clear();
    convergence["absolute"] = {};
    convergence["unique"] = {};
    JSON_MO_info.clear();
    JSON_SO_info.clear();
}

void RoundSchedule::run() {
    int round = 0;
    int lowestActiveGAIdx = 0;
    int highestActiveGAIdx = 0;
    bool optimumFound = false;
    bool done = false;
    long start = millis();
    long intermediateTime = millis();
    while (!done) {

        //Stopping conditions
        if (maxRounds != -1 && round >= maxRounds) {
            JSON_Run["stoppingCondition"] = "maxRoundsExceeded";
            break;
        } else if (maxSeconds != -1 && millis() - start > maxSeconds * 1000) {
            JSON_Run["stoppingCondition"] = "maxTimeExceeded";
            break;
        } else if (maxEvaluationsExceeded()){
            JSON_Run["stoppingCondition"] = "maxEvaluationsExceeded";
            break;
        } else if (maxUniqueEvaluationsExceeded()){
            JSON_Run["stoppingCondition"] = "maxUniqueEvaluationsExceeded";
            break;
        } else if (fitFunc->maxNetworkUniqueEvaluationsExceeded()){
            JSON_Run["stoppingCondition"] = "maxNetworkUniqueEvaluationsExceeded";
            break;
        }
        
        // Write an update every 10 minutes.
        if (printProgressOnIntervals && millis() - intermediateTime > (10 * 60 * 1000)){
            cout << "evals: " << fitFunc->totalEvaluations << "  uniqEvals: " << fitFunc->totalUniqueEvaluations << "  netUniqEvals: " << fitFunc->totalNetworkUniqueEvaluations << "  time: " << (millis() - start) / 1000 << endl;
            intermediateTime = millis();
        }

        //TODO: Loop only through active GA's, not until maxPopSizeLevel
        for (int i = lowestActiveGAIdx; i <= highestActiveGAIdx; i++) {

            //First check if GA is not terminated
            if (!gaList[i]->terminated) {

                //If this is the first non-terminated GA, always do the round
                if(i == lowestActiveGAIdx){
                    whichShouldRun[i] = 1;
                }

                //Check if this GA should be run
                if(whichShouldRun[i] == 1){
                    //If it is not the first non-terminated GA, don't run it in the next cycle
                    if(i != lowestActiveGAIdx){
                        whichShouldRun[i] = 0;
                    }

                    // Create a pointer to the current GA
                    ga = gaList[i];

                    // Initialize the GA if that has not been done yet
                    if(!ga->initialized){
                        ga->initialize();
                        JSON_Run["popsizereached"] = ga->populationSize;
                        
                        // Define the first ever individual as bestIndividualOverall
                        if(i == 0){
                            fitFunc->bestIndividual = ga->population[0].copy();
                        }
                        
                        ga->evaluateAll();
                        if(fitFunc->isMO()){
                            fitFunc->updateElitistArchive(ga->population);
                        } 
                    }
                    
                    // Do the round on this GA
//                    ga->print();
//                    cout << "Performing round with popsize " << ga->populationSize << endl;
                    ga->round();
                    if(printPopulationAfterRound) ga->print();
                    
                    if (maxEvaluationsExceeded() || maxUniqueEvaluationsExceeded() || fitFunc->maxNetworkUniqueEvaluationsExceeded()){
                        break;
                    }
                    
                    // If the current GA has found the optimum, break out of the loop
                    if (ga->isOptimal()){
//                        cout << "ga " << ga->populationSize << " is optimal" << endl;
                        if(printPopulationOnOptimum) ga->print();
                        optimumFound = true;
                        JSON_Run["successfulGAPopulation"] = ga->populationSize;
                        JSON_Run["successfulGARoundCount"] = ga->roundsCount;
                        break;

                    // Else if the GA is converged, terminate this GA and all before.
                    } else if (ga->isConverged()){
                        // GA.termination is already set to true inside isConverged()-method
                        // Set all previous GA's to terminated
//                        cout << "Terminate ga " << ga->populationSize << " due to convergence" << endl;
                        terminateGAs(i);
                        lowestActiveGAIdx = i + 1;
                        // Break out, because the GA with highest popSize has converged
                        if(lowestActiveGAIdx == maxPopSizeLevel){
                            done = true;
                            break;
                        } else {
                            // In case the ga converges before the next one has even started, make sure to start the next one.
                            highestActiveGAIdx = max(highestActiveGAIdx, i + 1);
                        }
                        
                        continue;

                    // Else if the previous GA has not terminated
                    } else if (i != 0 && !gaList[i-1]->terminated){
                        
                        // If this is a SO problem, check if this GA has a higher fitness than the previous GA. If so, terminate previous GA and all before.
                        if (fitFunc->numObjectives == 1 && ga->getAvgFitness()[0] > gaList[i-1]->getAvgFitness()[0]){
                            terminateGAs(i - 1);
                            lowestActiveGAIdx = i;
                        }
                        
                        // If this is a MO problem, check if this GA's first front dominates at least (X % + 1) solutions in the previous GA. If so, terminate previous GA and all before.
                        float percentageRequired = 0.5;
                        if (fitFunc->numObjectives > 1 && MOterminationCondition(ga, gaList[i-1], percentageRequired)){
//                            cout << "GA with " << ga->populationSize << " popsize dominates GA with " << gaList[i-1]->populationSize << " popsize." << endl;
                            terminateGAs(i - 1);
                            lowestActiveGAIdx = i;
                        }
                        
                    // Else if this GA had the highest population size and is terminated
                    } else if (i == maxPopSizeLevel - 1 && ga->terminated){
                        done = true;
                        JSON_Run["stoppingCondition"] = "terminated";
                        break;
                    }

                    // If this GA has run 4 times, make sure the next GA also does a run
                    if ((gaList[i]->roundsCount) % interval == 0){
                        whichShouldRun[i+1] = 1;
                        highestActiveGAIdx = min(max(highestActiveGAIdx, i+1), maxPopSizeLevel - 1);
                    }
                }
                
                JSON_Run["time_taken"] = millis() - start;
                JSON_Run["evals_total"] = fitFunc->totalEvaluations;
                JSON_Run["evals_unique"] = fitFunc->totalUniqueEvaluations;
                JSON_Run["evals_network_unique"] = fitFunc->totalNetworkUniqueEvaluations;
                if (storeConvergence)
                    JSON_Run["convergence"] = convergence;
                if(saveLogFilesOnEveryUpdate) Utility::writeRawData(JSON_Run.dump(), path_JSON_Run);

            }
        }

        if(optimumFound){
            JSON_Run["success"] = true;
            JSON_Run["stoppingCondition"] = "optimumReached";
            break;
        }

        //Increment counter
        round++;
    }
    
    long stop = millis();
    JSON_Run["time_taken"] = stop - start;
    JSON_Run["evals_total"] = fitFunc->totalEvaluations;
    JSON_Run["evals_unique"] = fitFunc->totalUniqueEvaluations;
    JSON_Run["evals_network_unique"] = fitFunc->totalNetworkUniqueEvaluations;
    if (storeConvergence)
        JSON_Run["convergence"] = convergence;
    
    return;
}

// Terminate ga's in gaList up to and including index n
void RoundSchedule::terminateGAs(int n){
    for(int i = 0; i < (n + 1); i++){
        (*gaList[i]).terminated = true;
    }
}

// Checks the conditions for termination. Terminate previous GA if either:
// 1 - The amount of solutions in the best front of previous GA that are dominated by solutions in current GA is more than ([percentageRequired] * #solutionsInPreviousGABestFront + 1).
// 2 - All solutions in the best front of previous GA are either dominated or appear in the best front of current GA.
bool RoundSchedule::MOterminationCondition(GA* gaCurrent, GA* gaPrev, float percentageRequired){
    
    int dominationCount = 0;
    int neededDominations = floor(percentageRequired * gaPrev->sortedPopulation[0].size()) + 1;
    int subsetCount = 0;
    
    for (Individual* indPrev : gaPrev->sortedPopulation[0]){
        for (Individual* indCur : gaCurrent->sortedPopulation[0]){
            // The set of dominated solutions and the set of solutions that are also in current GA's best front are disjoint.
            if (indCur->dominates(*indPrev)){
                dominationCount++;
                break;
            }
            
            if (indCur->fitnessEquals(*indPrev)){
                subsetCount++;
                break;
            }
        }
        // If the required amount of solutions is dominated, terminate previous GA.
        if (dominationCount >= neededDominations){
            return true;
        }
    }
    // If all solutions in previous GA's best front are dominated or are in the best front of current GA, then terminate previous GA.
    if (subsetCount + dominationCount == gaPrev->sortedPopulation[0].size()){
        return true;
    }
    return false;
}


int RoundSchedule::getAmountOfEvaluations(){
    int evaluations = 0;
    for (GA *ga : gaList) {
        evaluations += ga->getTotalAmountOfEvaluations();
    }
    return evaluations;
}

bool RoundSchedule::maxEvaluationsExceeded() {
    return fitFunc->maxEvaluationsExceeded();
}

bool RoundSchedule::maxUniqueEvaluationsExceeded() {
    return fitFunc->maxUniqueEvaluationsExceeded();
}

void RoundSchedule::writeOutputGenerationCSV(string filename){
    std::ofstream myfile;
    myfile.open(filename);
    for (int i = 0; i < ga->population.size(); i++){
        myfile << ga->population[i].toString(ga->population[i].genotype);
        for (int j = 0; j < fitFunc->numObjectives; j++){
            myfile << "," << to_string(ga->population[i].fitness[j]);
        }
        myfile << "\n";
    }
    myfile.close();
}
