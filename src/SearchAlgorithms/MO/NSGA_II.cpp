//
//  NSGA_II.cpp
//  GA
//
//  Created by Tom den Ottelander on 03/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "NSGA_II.hpp"

using namespace std;

NSGA_II::NSGA_II(FitnessFunction * fitFunc) : NSGA_II::NSGA_II(fitFunc, new TwoPointCrossover(), 1.0f, (1.0f / fitFunc->totalProblemLength)){}

NSGA_II::NSGA_II(FitnessFunction * fitFunc, Variation * var, float crossoverProbability, float mutationProbability) : SimpleGA(fitFunc, var, NULL),
    crossoverProbability(crossoverProbability),
    mutationProbability(mutationProbability) {
        cout << "Variation: " << var->id() << endl;
        cout << "CrossoverProbability: " << crossoverProbability << endl;
        cout << "MutationProbability: " << mutationProbability << endl;
}

void NSGA_II::round() {
    // Do an initial non dominating sorting (rank assigning) and crowding distance assigning at the start of the first round
    if(roundsCount == 0){
        sortedPopulation = nonDominatedSorting(population);
        fitFunc_ptr->updateElitistArchive(sortedPopulation[0]);
        parentPop = truncate(sortedPopulation);
    }
    
    // Do tournament selection on the truncated parent population to create a child population
    vector<Individual> childPop = selection(parentPop);
    
    // Mutate the child population and evaluate all resulting individuals
    mutation(childPop);
    
    evaluateAll(childPop);
    if(fitFunc_ptr->isDone()){
        return;
    }
    
    // Merge Pt and Qt
    vector<Individual> mergedPopulation = merge(parentPop, childPop);

    // Sort the merged population on rank
    sortedPopulation = nonDominatedSorting(mergedPopulation);
    
    // Select Pt by truncating based on rank and crowding distance
    parentPop = truncate(sortedPopulation);
    population = parentPop;
    
    roundsCount++;
}

void NSGA_II::clearMOinformation(vector<Individual> &population){
    for (int i = 0; i < population.size(); i++){
        population[i].clearMOinformation();
    }
}

// Sorts the individuals into fronts. If n is not -1, then there process of sorting into fronts stops when
// finding all the solutions of a front is done and the total amount of solutions is at least n.
vector<vector<Individual*>> NSGA_II::nonDominatedSorting (vector<Individual> &population, int n){
    
    // Clear the information like front and crowding distance from the population.
    clearMOinformation(population);
    // Loop over every individual combination to see which individuals dominate which other individuals.
    for (int i = 0; i < population.size(); i++){
        for (int j = 0; j < population.size(); j++){
            if (population[i].dominates(population[j])){
                // Individual i dominates individual j
                population[j].dominationCount += 1;
                population[i].dominationList.push_back(&population[j]);
            }
        }
    }
    
    // Put a pointer to every individual in a pool that we can iterate over and modify.
    vector<Individual*> pool;
    for (int i = 0; i < population.size(); i++){
        pool.push_back(&population[i]);
    }
    
    sortedPopulation.clear();
    int frontIdx = 0;
    int individualCount = 0;
    // See which individuals have domination count 0. Put these in the same front.
    while (!pool.empty()) {
        vector<Individual*> remainder_pool;
        vector<Individual*> front;
        for (int i = 0; i < pool.size(); i++){
            if(pool[i]->dominationCount == 0){
                pool[i]->front = frontIdx;
                front.push_back(pool[i]);
                individualCount++;
            } else {
                remainder_pool.push_back(pool[i]);
            }
        }
        
        // Modify domination count of other individuals by subtracting the dominates of dominated solutions by solutions from this front.
        for (Individual* can : front){
            for (Individual *dominatingIndividual : can->dominationList){
                dominatingIndividual->dominationCount -= 1;
            }
        }
        
        sortedPopulation.push_back(front);
        frontIdx++;
        // If we only need [n] individuals, we can stop assigning fronts.
        if(n != -1 && individualCount >= n){
            break;
        }
        pool = remainder_pool;
    }
    return sortedPopulation;
}

// Sorts the individuals in a front based on the crowding distance.
void NSGA_II::CrowdingDistanceSorting (vector<Individual*> &front){
    
    if(front.size() <= 2){
        for (int i = 0; i < front.size(); i++){
            front[i]->crowdingDistance = INFINITY;
        }
        return;
    }
    
    for (int obj = 0; obj < fitFunc_ptr->numObjectives; obj++){
        
//        cout << "UNSORTED FRONT: " << endl;
//        for (int i = 0; i < front.size(); i++) cout << i << ": " << front[i]->toString() << endl;

        // Sort individuals in this front on fitness for objective [obj].
        vector<int> indices = quickSort(front, obj);
        
//        cout << "INDICES SORTED: " << endl;
//        for (int i = 0; i < front.size(); i++) cout << indices[i] << endl;
        
        //Comment out this line below to match the original implementation by Deb.
        front[indices[0]]->crowdingDistance = INFINITY;
        front[indices[indices.size()-1]]->crowdingDistance = INFINITY;
        
        if (front.size() > 2){
            float maxObjFitness = front[indices[front.size()-1]]->fitness[obj];
            float minObjFitness = front[indices[0]]->fitness[obj];
            float normalizationValue = maxObjFitness - minObjFitness;
            
            for (int i = 1; i < front.size() - 1; i++){
                if(front[indices[i]]->crowdingDistance != INFINITY){
                    float prevObjFitness = front[indices[i-1]]->fitness[obj];
                    float nextObjFitness = front[indices[i+1]]->fitness[obj];
                    float newCrowdingDistance;
                    if (normalizationValue == 0){
                        newCrowdingDistance = INFINITY;
                    } else {
                        newCrowdingDistance = front[indices[i]]->crowdingDistance + ((nextObjFitness - prevObjFitness) / normalizationValue);
                    }
                    front[indices[i]]->crowdingDistance = newCrowdingDistance;
                }
            }
        }
        
//        cout << "SORTED FRONT: " << endl;
//        for (int i = 0; i < front.size(); i++) cout << i << ": " << front[indices[i]]->toString() << endl
        
    }
    
    for (int i = 0; i < front.size(); i++){
        if(front[i]->crowdingDistance != INFINITY){
            front[i]->crowdingDistance = front[i]->crowdingDistance / fitFunc_ptr->numObjectives;
        }
    }
    
//    cout << "FINAL FRONT: " << endl;
//    for (int i = 0; i < front.size(); i++) cout << i << ": " << front[i]->toString() << endl;
    
    sort(front.begin(), front.end(), [](const Individual* rhs, const Individual* lhs){
        return lhs->crowdingDistance < rhs->crowdingDistance;
    });
}

void NSGA_II::insertionSort(vector<Individual*> &front, int objectiveIdx){
    for (int i = 0; i < front.size(); i++){
        int moveIdx = i;
        while (moveIdx != 0){
            if (front[moveIdx]->fitness[objectiveIdx] <= front[moveIdx-1]->fitness[objectiveIdx]){
                Individual* temp = front[moveIdx-1];
                front[moveIdx-1] = front[moveIdx];
                front[moveIdx] = temp;
                moveIdx--;
            } else {
                break;
            }
        }
    }
}

// Quicksort. Sorts the array of indices based on their correspondence with individuals in the front.
vector<int> NSGA_II::quickSort(vector<Individual*> &front, int objectiveIdx){
    vector<int> indices = Utility::getOrderedArray(front.size(), Utility::Order::ASCENDING);
    quickSort(front, indices, objectiveIdx, 0, front.size() - 1);
    return indices;
}

// Subroutine for quicksort
void NSGA_II::quickSort(vector<Individual*> &front, vector<int> &indices, int objectiveIdx, int left, int right){
    if (left < right){
        int index = Utility::getRand(left, right);
        
        int temp = indices[right];
        indices[right] = indices[index];
        indices[index] = temp;
        
        float pivot = front[indices[right]]->fitness[objectiveIdx];
        int i = left - 1;
        for (int j = left; j < right; j++){
            if (front[indices[j]]->fitness[objectiveIdx] <= pivot){
                i += 1;
                temp = indices[j];
                indices[j] = indices[i];
                indices[i] = temp;
            }
        }
        index = i + 1;
        temp = indices[index];
        indices[index] = indices[right];
        indices[right] = temp;
        quickSort(front, indices, objectiveIdx, left, index - 1);
        quickSort(front, indices, objectiveIdx, index + 1, right);
    }
}

// Returns true if can1 is favoured over can2
bool NSGA_II::crowdComparisonOperator(const Individual &lhs, const Individual &rhs){
    if (lhs.front < rhs.front){
        return true;
    } else if (lhs.front > rhs.front){
        return false;
    } else if (lhs.crowdingDistance > rhs.crowdingDistance){
        return true;
    } else if (lhs.crowdingDistance < rhs.crowdingDistance){
        return false;
    } else if (Utility::getRand() < 0.5) {
        return true;
    } else {
        return false;
    }
}

// Tournament with size 2
Individual NSGA_II::tournament(Individual &ind1, Individual &ind2){
    if (crowdComparisonOperator(ind1, ind2)){
        return ind1;
    } else {
        return ind2;
    }
}

// Fill a new population by selecting from a population first based on front and then on crowding distance.
vector<Individual> NSGA_II::truncate(vector<vector<Individual*>> sortedPopulation){
    vector<Individual> parentPop;
    parentPop.reserve(populationSize);
    int individualsAdded = 0;
    
    for (vector<Individual*> &front : sortedPopulation){
        CrowdingDistanceSorting(front);
        for (int i = 0; i < front.size(); i++){
            parentPop.push_back(front[i]->copy());
            individualsAdded++;
            if (individualsAdded == populationSize){
                return parentPop;
            }
        }
    }
    return parentPop;
}

vector<Individual> NSGA_II::selection(vector<Individual> parentPop){
    vector<Individual> childPop;
    childPop.reserve(populationSize);

    if (populationSize % 4 != 0){
//        cout << "Consider setting the population size to a multiple of 4." << endl;
    }
    
    int index = 0;
    vector<int> randIdxArray = Utility::getOrderedArray(populationSize, Utility::Order::RANDOM);
    for (int i = 0; i < populationSize; i+=2){
        if (index+3 >= populationSize){
            randIdxArray = Utility::getOrderedArray(populationSize, Utility::Order::RANDOM);
            index = 0;
        }

        // Apply tournament selection with size 2
        Individual parent1 = tournament(parentPop[randIdxArray[index]], parentPop[randIdxArray[index+1]]);
        Individual parent2 = tournament(parentPop[randIdxArray[index+2]], parentPop[randIdxArray[index+3]]);
        
        // Combine winners of the tournament selections by doing crossover.
        pair<Individual, Individual> offspring;
        if (Utility::getRand() < crossoverProbability){
            offspring = variation_ptr->crossover(parent1, parent2);
        } else {
            offspring = pair<Individual, Individual> {parent1.copy(), parent2.copy()};
        }
        childPop.push_back(offspring.first);
        
        if (i+1 < populationSize){
            childPop.push_back(offspring.second);
        }
        
        index += 4;
    }

    return childPop;
}

void NSGA_II::mutation(vector<Individual> &children){
    Variation::mutate(children, mutationProbability, fitFunc_ptr->problemType->alphabet);
}

vector<Individual> NSGA_II::merge(vector<Individual> &parentPop, vector<Individual> &childPop){
    vector<Individual> mergedPop;
    for (int i = 0; i < parentPop.size(); i++){
        mergedPop.push_back(parentPop[i].copy());
    }
    for (int i = 0; i < childPop.size(); i++){
        mergedPop.push_back(childPop[i].copy());
    }
    return mergedPop;
}

bool NSGA_II::isDiverse() {
    // Since there is mutation in this algorithm, diversity will always be ensured.
    return true;
}

GA* NSGA_II::clone() const {
    return new NSGA_II(static_cast<const NSGA_II&>(*this));
}

string NSGA_II::id(){
    return "NSGA-II_xo=" + variation_ptr->id();
}
