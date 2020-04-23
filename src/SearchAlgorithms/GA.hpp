//
//  GA.hpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef GA_hpp
#define GA_hpp

#include "Selection.hpp"
#include "Variation.hpp"
#include "FitnessFunction.hpp"
#include "Individual.hpp"

#include <vector>
#include <string>

class GA {
public:
    int populationSize;
    FitnessFunction * fitFunc_ptr;
    Selection * selection_ptr;
    Variation * variation_ptr;
    std::vector<Individual> population;
    std::vector<std::vector<Individual*>> sortedPopulation;
    bool terminated;
    bool converged = false;
    bool initialized;
    int roundsCount;
    bool isLocalSearchAlgorithm = false;
    bool isRandomSearchAlgorithm = false;
    int noAdditionToElitistArchiveCount = 0;
    bool preventIMS = false;
    
    GA(FitnessFunction * fitfunc);
    
    virtual void initialize();
    
    void run();
    virtual void round();
    
    void roundReplacementVariationSelection();
    void roundPOVariationSelection();
    
    void evaluateAll(std::vector<Individual> &population);
    void evaluateAll();
    bool isConverged();
    virtual bool isDiverse();
    bool isOptimal();
    std::vector<float> getAvgFitness();
    int getTotalAmountOfEvaluations();
    
    void initializeTrueRandomPopulation();
    void initializeNASPopulation();
    
    std::string toString();
    std::string populationToString(std::vector<Individual> &population);
    void print();
    
    void setPopulationSize (int n);
    void setFitnessFunction (FitnessFunction * fitfunc);
    void setSelection (Selection * sel);
    void setVariation (Variation * var);
    void setPointers(FitnessFunction * fitfunc, Selection * sel, Variation * var);
    
    virtual GA* clone() const;
    
    virtual std::string id();
};

#endif /* GA_hpp */
