//
//  FitnessFunction.cpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "FitnessFunction.hpp"

using namespace std;
using namespace nlohmann;

extern bool storeAbsoluteConvergence;
extern bool storeUniqueConvergence;
extern bool printElitistArchiveOnUpdate;
extern bool storeDistanceToParetoFrontOnElitistArchiveUpdate;
extern nlohmann::json convergence;
extern bool saveLogFilesOnEveryUpdate;
extern string path_JSON_MO_info;
extern string path_JSON_SO_info;
extern string path_JSON_Run;
extern string dataDir;
extern nlohmann::json JSON_MO_info;
extern nlohmann::json JSON_SO_info;
extern nlohmann::json JSON_Run;
extern bool storeElitistArchive;
extern bool updateElitistArchiveOnEveryEvaluation;
extern int loggingIntervalMode;
extern int loggingLinearInterval;
extern bool printEveryEvaluation;

bool errorParetoFrontSent = false;
bool errorParetoPointsSent = false;

/* ------------------------ Base Fitness Function ------------------------ */

FitnessFunction::FitnessFunction(vector<float> optimum, ProblemType *problemType) :
    bestIndividual(),
    optimum(optimum),
    optimumFound(false),
    maxEvaluations(-1),
    maxUniqueEvaluations(-1),
    maxNetworkUniqueEvaluations(-1),
    problemType(problemType),
    totalEvaluations(0),
    totalUniqueEvaluations(0),
    totalNetworkUniqueEvaluations(0),
    uniqueSolutions()
{
}

FitnessFunction::FitnessFunction(ProblemType *problemType) :
    bestIndividual(),
    optimumFound(false),
    maxEvaluations(-1),
    maxUniqueEvaluations(-1),
    maxNetworkUniqueEvaluations(-1),
    problemType(problemType),
    totalEvaluations(0),
    totalUniqueEvaluations(0),
    totalNetworkUniqueEvaluations(0),
    uniqueSolutions()
{
}

void FitnessFunction::clear(){
    bestIndividual = Individual();
    elitistArchive.clear();
    optimumFound = false;
    totalEvaluations = 0;
    totalUniqueEvaluations = 0;
    totalNetworkUniqueEvaluations = 0;
    uniqueSolutions.clear();
    JSON_MO_info.clear();
    JSON_SO_info.clear();
    done = false;
    networkLibrary.clear();
}

// Performs additional operations like incrementing the amount of (unique) evaluations, checking whether an individual is the best so far yet and storing convergence data.
void FitnessFunction::evaluationProcedure(Individual &ind){
//    cout << "Evaluating " << ind.toString() << "at eval=" << totalEvaluations << " and uniqEval=" << totalUniqueEvaluations << endl;
    checkIfBestFound(ind);
    totalEvaluations++;
    
    // Do this stuff only if it is a MO-problem.
    if(numObjectives > 1){
        
        // Update the elitist archive
        if(updateElitistArchiveOnEveryEvaluation){
            updateElitistArchive(&ind);
        }
        
        // Store the distance of the front to the approximation on every log10 interval.
        if(log(totalEvaluations)){
            pair<float, float> avg_max_distance = calculateDistanceParetoToApproximation();
            JSON_MO_info["changes_on_interval"]["total_evals"]["elitist_archive"].push_back(elitistArchiveToJSON());
            JSON_MO_info["changes_on_interval"]["total_evals"]["avg_dist"].push_back(avg_max_distance.first);
            JSON_MO_info["changes_on_interval"]["total_evals"]["max_dist"].push_back(avg_max_distance.second);
            JSON_MO_info["changes_on_interval"]["total_evals"]["evals"].push_back(totalEvaluations);
            JSON_MO_info["changes_on_interval"]["total_evals"]["pareto_points_found"].push_back(paretoPointsFound());
        }
    }
    
    // Do this stuff only if it is a SO-problem
    if (numObjectives == 1){
        
        if(log(totalEvaluations)){
            JSON_SO_info["changes_on_interval"]["total_evals"]["evaluated_solution_genotype"].push_back(Utility::genotypeToString(ind.genotype));
            JSON_SO_info["changes_on_interval"]["total_evals"]["evaluated_solution_fitness"].push_back(ind.fitness[0]);
            JSON_SO_info["changes_on_interval"]["total_evals"]["best_solution_genotype"].push_back(Utility::genotypeToString(bestIndividual.genotype));
            JSON_SO_info["changes_on_interval"]["total_evals"]["best_solution_fitness"].push_back(bestIndividual.fitness[0]);
            JSON_SO_info["changes_on_interval"]["total_evals"]["evals"].push_back(totalEvaluations);
        }
        
        // Store the best fitness found so far
        if(storeAbsoluteConvergence){
            convergence["absolute"].push_back(bestIndividual.fitness[0]);
        }
    }
    
    // Do stuff based on whether this is a new unique evaluation
    if(!uniqueSolutions.contains(ind.genotype)){
        uniqueSolutions.put(ind.genotype, ind.fitness);
        totalUniqueEvaluations++;
        
        // Store unique convergence only for SO-problems
        if(numObjectives == 1){
            if(storeUniqueConvergence){
                convergence["unique"].push_back(bestIndividual.fitness[0]);
            }
            
            if(log(totalUniqueEvaluations)){
                JSON_SO_info["changes_on_interval"]["unique_evals"]["evaluated_solution_genotype"].push_back(Utility::genotypeToString(ind.genotype));
                JSON_SO_info["changes_on_interval"]["unique_evals"]["evaluated_solution_fitness"].push_back(ind.fitness[0]);
                JSON_SO_info["changes_on_interval"]["unique_evals"]["best_solution_genotype"].push_back(Utility::genotypeToString(bestIndividual.genotype));
                JSON_SO_info["changes_on_interval"]["unique_evals"]["best_solution_fitness"].push_back(bestIndividual.fitness[0]);
                JSON_SO_info["changes_on_interval"]["unique_evals"]["evals"].push_back(totalUniqueEvaluations);
            }
        }
        
        // Store distance front to approximation only if MO-problem and if unique evaluations is on a log10 interval.
        if(numObjectives > 1 && log(totalUniqueEvaluations)){
            pair<float, float> avg_max_distance = calculateDistanceParetoToApproximation();
            JSON_MO_info["changes_on_interval"]["unique_evals"]["elitist_archive"].push_back(elitistArchiveToJSON());
            JSON_MO_info["changes_on_interval"]["unique_evals"]["avg_dist"].push_back(avg_max_distance.first);
            JSON_MO_info["changes_on_interval"]["unique_evals"]["max_dist"].push_back(avg_max_distance.second);
            JSON_MO_info["changes_on_interval"]["unique_evals"]["evals"].push_back(totalUniqueEvaluations);
            JSON_MO_info["changes_on_interval"]["unique_evals"]["pareto_points_found"].push_back(paretoPointsFound());
        }
    }
    
    if(printEveryEvaluation){
        cout << "  Eval: " << Utility::padWithSpacesAfter(to_string(totalEvaluations), 8);
        cout << "UniqEval: " << Utility::padWithSpacesAfter(to_string(totalUniqueEvaluations), 8);
        cout << "NetworkUniqEval: " << Utility::padWithSpacesAfter(to_string(totalNetworkUniqueEvaluations), 8);
        cout << "Ind: " << ind.toString();
        if (storeNetworkUniqueEvaluations){
            try{
                cout << "    Network: " << networkLibrary.hash(ind.genotype);
            } catch (exception) {
                // Do nothing.
            }
        }
        cout << endl;
    }
    
    if(saveLogFilesOnEveryUpdate){
        try{
            Utility::writeRawData(JSON_Run.dump(), path_JSON_Run);
            if(numObjectives > 1)
                Utility::writeRawData(JSON_MO_info.dump(), path_JSON_MO_info);
            else
                Utility::writeRawData(JSON_SO_info.dump(), path_JSON_SO_info);
        } catch (exception) {
            cout << "Could not save results." << endl;
        }
    }
}

bool FitnessFunction::log(int evals){
    switch (loggingIntervalMode) {
        case 0: return Utility::isLogPoint(evals, 2);
        case 1: return Utility::isLinearPoint(evals, loggingLinearInterval);
        default:
            cout << "Logging interval mode not set correctly." << endl;
            return false;
    }
}

bool FitnessFunction::isDone(){
    if(done) {
        return true;
    }
    
    if(optimumFound || maxEvaluationsExceeded() || maxUniqueEvaluationsExceeded() || maxNetworkUniqueEvaluationsExceeded()){
        done = true;
    }
    
    return done;
}

bool FitnessFunction::isMO(){
    if(numObjectives > 1){
        return true;
    }
    return false;
}

// Displays the description of the fitness function
void FitnessFunction::display(){
    cout << "Base fitness function" << endl;
}

// Checks whether this individual is fitter than the best found individual so far.
// Checks whether the individual is optimal.
void FitnessFunction::checkIfBestFound(Individual &ind){
    
    if(optimumFound){
        return;
    }
    
    if(convergenceCriteria == ConvergenceCriteria::OPTIMAL_FITNESS){
        optimumFound = true;
        for (int obj = 0; obj < optimum.size(); obj++){
            if (ind.fitness[obj] < optimum[obj] || optimum[obj] == -1){
                optimumFound = false;
                break;
            }
        }
    }
    
    else if (convergenceCriteria == ConvergenceCriteria::GENOTYPE){
        if(ind.genotypeEquals(optimalGenotype)){
            optimumFound = true;
        };
    }
    
    // Set bestIndividual only for SO-problems.
    if(numObjectives == 1 && ind.fitness[0] > bestIndividual.fitness[0]){
        bestIndividual = ind.copy();
    }
}


// Update the eltist archive by supplying the best front found. It adds non-dominated solution to and removes dominated solutions from the archive. Returns true if the supplied front adds any individuals to the elitist archive
bool FitnessFunction::updateElitistArchive(Individual* ind){
    
    bool addToArchive = true;
    
    // Loop over every solution that is in the archive at the beginning of this method.
    for (int i = elitistArchive.size() - 1; i >= 0; i--){
        
        // Delete every solution from the archive that is dominated by this solution.
        if(ind->dominates(elitistArchive[i])){
            elitistArchive.erase(elitistArchive.begin() + i);
        }
        
        // If the solution from the archive dominates this one or it is equal, then don't add this solution to the archive. Thus, remove it from the front.
        else if (elitistArchive[i].dominates(*ind) || ind->fitnessEquals(elitistArchive[i])){
            addToArchive = false;
            return addToArchive;
        }
    }
    
    if (addToArchive){
//        cout << "Add to archive: " << ind->toString() << endl;
        elitistArchive.push_back(ind->copy());
    
        // Only check for convergence criteria if there are new solutions added to the elitist archive.
        pair<float, float> avg_max_distance = {-1, -1};
        
        if (convergenceCriteria == ConvergenceCriteria::ENTIRE_PARETO){
            // If the true pareto front is not known, rely on enitreParetoFrontFound() functions that are implemented in child classes
            if (trueParetoFront.size() == 0){
                if(entireParetoFrontFound()){
                    optimumFound = true;
                }
            // Else, compute the avg distance of front to approximation and check if it is 0. (in fact if it is < 0.000001).
            } else {
                if (avg_max_distance.first == -1)
                    avg_max_distance = calculateDistanceParetoToApproximation();
                
                if (avg_max_distance.first < 0.000001){
                    optimumFound = true;
                }
            }
        }
        
        else if (convergenceCriteria == ConvergenceCriteria::EPSILON_PARETO_DISTANCE){
            if (avg_max_distance.first == -1)
                avg_max_distance = calculateDistanceParetoToApproximation();
            
            if (avg_max_distance.first <= epsilon){
                optimumFound = true;
            }
        }
        
        else if (convergenceCriteria == ConvergenceCriteria::NONE){
            // Do nothing
        }
        
        if (printElitistArchiveOnUpdate){
            drawElitistArchive();
        }
    }
    
    return addToArchive;
}

bool FitnessFunction::updateElitistArchive(vector<Individual> &front){
    bool solutionsAdded = false;
    for (int i = 0; i < front.size(); i++){
        bool added = updateElitistArchive(&front[i]);
        solutionsAdded = solutionsAdded || added;
    }
    return solutionsAdded;
}

bool FitnessFunction::updateElitistArchive(vector<Individual*> &front){
    bool solutionsAdded = false;
    for (int i = 0; i < front.size(); i++){
        bool added = updateElitistArchive(front[i]);
        solutionsAdded = solutionsAdded || added;
    }
    return solutionsAdded;
}

// The elitist archive is used as approximation
// Returns two values in the pair:
//  value 1:  the average of the distances from the points on the true pareto to the closest point in the elitist archive
//  value 2:  the maximum of the distances from the points on the true pareto to the closest point in the elitist archive
pair<float, float> FitnessFunction::calculateDistanceParetoToApproximation(){
    if (trueParetoFront.size() == 0){
        if (!errorParetoFrontSent) {
            cout << "ERROR: Wanted to calculate distance of pareto to approximation, but true Pareto front is unknown" << endl;
            errorParetoFrontSent = true;
        }
        storeDistanceToParetoFrontOnElitistArchiveUpdate = false;
        return {-1.0f, -1.0f};
    }
    
    if (elitistArchive.size() == 0){
        return {INFINITY, INFINITY};
    }
    
    float summedDistance = 0.0f;
    float maxDistance = 0.0f;
    
    for (int i = 0; i < trueParetoFront.size(); i++){
        float minimalDistance = INFINITY;
        for (int j = 0; j < elitistArchive.size(); j++){
            float distance = Utility::EuclideanDistance(trueParetoFront[i], elitistArchive[j].fitness);
            if (distance < minimalDistance){
                minimalDistance = distance;
                if (distance == 0.0f){
                    break;
                }
            }
        }
        summedDistance += minimalDistance;
        maxDistance = max(maxDistance, minimalDistance);
    }
    
    float avgDistance = summedDistance / trueParetoFront.size();
//    distanceToParetoFrontData.push_back(tuple<int, int, float>{totalEvaluations, totalUniqueEvaluations, avgDistance});
    return {avgDistance, maxDistance};
}

int FitnessFunction::paretoPointsFound(){
    if(trueParetoFront.size() == 0){
        if (!errorParetoPointsSent){
            cout << "No Pareto points known" << endl;
            errorParetoPointsSent = true;
        }
        return 0;
    }
    
    int pointsFound = 0;
    
    for (int i = 0; i < trueParetoFront.size(); i++){
        vector<float> paretoPoint = trueParetoFront[i];
        for (int j = 0; j < elitistArchive.size(); j++){
            if (elitistArchive[j].fitness[0] == paretoPoint[0]
                && elitistArchive[j].fitness[1] == paretoPoint[1]){
                pointsFound++;
                break;
            }
        }
    }
    return pointsFound;
}

json FitnessFunction::elitistArchiveToJSON(){
    json result;
    for (int i = 0; i < elitistArchive.size(); i++){
        json solution;
        solution["g"] = Utility::genotypeToString(elitistArchive[i].genotype);
        
        json array;
        array[0] = elitistArchive[i].fitness[0];
        array[1] = elitistArchive[i].fitness[1];
        solution["f"] = array;
        
        result[i] = solution;
    }
    return result;
}

// Override this method in specific problems. For example, do an extra check on objective values.
bool FitnessFunction::entireParetoFrontFound(){
    return elitistArchive.size() == optimalParetoFrontSize;
}

void FitnessFunction::setGenotypeChecking(vector<int> genotype){
    checkForGenotype = true;
    optimalGenotype = genotype;
}

// Returns the total amount of evaluations over all fitness functions.
int FitnessFunction::getTotalAmountOfEvaluations(){
    return totalEvaluations;
}

// Checks whether the maximum amount of evaluations is exceeded
bool FitnessFunction::maxEvaluationsExceeded(){
    return totalEvaluations >= maxEvaluations && maxEvaluations != -1;
}

bool FitnessFunction::maxUniqueEvaluationsExceeded(){
    return totalUniqueEvaluations >= maxUniqueEvaluations && maxUniqueEvaluations != -1;
}

bool FitnessFunction::maxNetworkUniqueEvaluationsExceeded(){
    return totalNetworkUniqueEvaluations >= maxNetworkUniqueEvaluations && maxNetworkUniqueEvaluations != -1;
}

// Returns the id of the fitness function
string FitnessFunction::id() {
    return "base";
}

// Sets the length and the optimum
void FitnessFunction::setLength(int length){
    totalProblemLength = length;
}

void FitnessFunction::setNumObjectives(int numObj){
    numObjectives = numObj;
}

// Sets the MO-problem optimum.
void FitnessFunction::setOptimum(vector<float> opt){
    optimum = opt;
}

// Sets the SO-problem optimum.
void FitnessFunction::setOptimum(float opt){
    optimum[0] = opt;
}

// Transforms the genotype (e.g. remove identity layers in encoding)
// Should be overridden in derived classes
vector<int> FitnessFunction::transform(vector<int> &genotype){
    return genotype;
}

void FitnessFunction::draw2DVisualization(vector<Individual> &population, int maxX, int maxY){
    vector<Individual*> drawList;
    drawList.reserve(population.size());
    for (int i = 0; i < population.size(); i++){
        drawList.push_back(&population[i]);
    }
    sort(drawList.begin(), drawList.end(), [](const Individual* lhs, const Individual* rhs){
        if (lhs->fitness[1] < rhs->fitness[1]){
            return true;
        } else if (lhs->fitness[1] > rhs->fitness[1]){
            return false;
        } else {
            return lhs->fitness[0] < rhs->fitness[0];
        }
    });
    int i = 0;
    string result = "";
    for (int y = 0; y < maxY; y++){
        for (int x = 0; x < maxX; x++){
            if (drawList[i]->fitness[0] == x && drawList[i]->fitness[1] == y){
                result += " o ";
            } else {
                result += " . ";
            }
            result += " ";
            while (drawList[i]->fitness[0] == x && drawList[i]->fitness[1] == y && i < drawList.size()-1){
                i++;
            }
        }
        result += "\n";
    }
    cout << result << endl;
}

void FitnessFunction::drawElitistArchive(){
    draw2DVisualization(elitistArchive, optimum[0]+1, optimum[1]+1);
}

void FitnessFunction::saveElitistArchiveToJSON(){
    json result;
    for (int i = 0; i < elitistArchive.size(); i++){
        json solution;
        solution["genotype"] = Utility::genotypeToString(elitistArchive[i].genotype);
        json fitness;
        for (int j = 0; j < numObjectives; j++){
            fitness[j] = elitistArchive[i].fitness[j];
        }
        solution["fitness"] = fitness;
        result[i] = solution;
    }
    Utility::writeRawData(result.dump(), dataDir, "");
}

void FitnessFunction::printElitistArchive(bool fullArchive){
    bool printDots = true;
    cout << "Elitist archive:   (size=" << elitistArchive.size() << ")" << endl;
    for (int i = 0; i < elitistArchive.size(); i++){
        if(fullArchive || i == 0 || i == 1 || i == elitistArchive.size() - 1){
            cout << i << ": " << elitistArchive[i].toString() << endl;
        } else if (printDots){
            cout << "..." << endl;
            printDots = false;
        }
    }
}

void FitnessFunction::logNetworkUniqueEvaluations(){
    if (numObjectives == 1){
        JSON_SO_info["changes_on_interval"]["network_unique_evals"]["best_solution_genotype"].push_back(Utility::genotypeToString(bestIndividual.genotype));
        JSON_SO_info["changes_on_interval"]["network_unique_evals"]["best_solution_fitness"].push_back(bestIndividual.fitness[0]);
        JSON_SO_info["changes_on_interval"]["network_unique_evals"]["evals"].push_back(totalNetworkUniqueEvaluations);
        if(saveLogFilesOnEveryUpdate) Utility::writeRawData(JSON_SO_info.dump(), path_JSON_SO_info);
    } else {
        pair<float, float> avg_max_distance = calculateDistanceParetoToApproximation();
        JSON_MO_info["changes_on_interval"]["network_unique_evals"]["elitist_archive_fitness"].push_back(elitistArchiveToJSON());
        JSON_MO_info["changes_on_interval"]["network_unique_evals"]["avg_dist"].push_back(avg_max_distance.first);
        JSON_MO_info["changes_on_interval"]["network_unique_evals"]["max_dist"].push_back(avg_max_distance.second);
        JSON_MO_info["changes_on_interval"]["network_unique_evals"]["evals"].push_back(totalNetworkUniqueEvaluations);
        JSON_MO_info["changes_on_interval"]["network_unique_evals"]["pareto_points_found"].push_back(paretoPointsFound());
        if(saveLogFilesOnEveryUpdate) Utility::writeRawData(JSON_MO_info.dump(), path_JSON_MO_info);
    }
}



/* ------------------------ OneMax Fitness Function ------------------------ */

OneMax::OneMax(int length) : FitnessFunction(vector<float>(1, length), getProblemType()) {}
OneMax::OneMax() : FitnessFunction(getProblemType()) {}

vector<float> OneMax::evaluate(Individual &ind) {
    int summedGenotype = accumulate(ind.genotype.begin(), ind.genotype.end(), 0);
    vector<float> result(1, summedGenotype);
    ind.fitness[0] = result[0];
    
    evaluationProcedure(ind);
    
    return result;
}

void OneMax::display() {
    cout << "OneMax fitness function" << endl;
}

string OneMax::id() {
    return "OM";
}

ProblemType* OneMax::getProblemType(){
    return new BinaryProblemType();
}

FitnessFunction* OneMax::clone() const {
    FitnessFunction* result = new OneMax(static_cast<const OneMax&>(*this));
    result->problemType = this->problemType;
    return result;
}


/* ------------------------ Leading Ones Fitness Function ------------------------ */

LeadingOnes::LeadingOnes(int length) : FitnessFunction(vector<float>(1, length), getProblemType()) {}
LeadingOnes::LeadingOnes() : FitnessFunction(getProblemType()) {}

ProblemType* LeadingOnes::getProblemType(){
    return new BinaryProblemType();
}

vector<float> LeadingOnes::evaluate(Individual &ind) {
    vector<float> result (1, 0);
    for (unsigned long i = 0; i < ind.genotype.size(); i++){
        if (ind.genotype[i] == 0){
            break;
        } else {
            result[0]++;
        }
    }
    ind.fitness[0] = result[0];
    
    evaluationProcedure(ind);
    
    return result;
}

FitnessFunction* LeadingOnes::clone() const {
    FitnessFunction* result = new LeadingOnes(static_cast<const LeadingOnes&>(*this));
    result->problemType = this->problemType;
    return result;
}

void LeadingOnes::display() {
    cout << "LeadingOnes fitness function" << endl;
}

string LeadingOnes::id() {
    return "LO";
}
