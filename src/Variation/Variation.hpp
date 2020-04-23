//
//  Variation.hpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef Variation_hpp
#define Variation_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include "Individual.hpp"
#include "Utility.hpp"
#include "FitnessFunction.hpp"
#include "FOS.hpp"

class Variation {
public:
    virtual std::vector<Individual> variate(std::vector<Individual> &population);
    virtual std::pair<Individual, Individual> crossover(Individual &ind1, Individual &ind2);
    virtual void display();
    virtual std::string id();
    
    static void mutate(Individual &ind, float mutationRate, std::vector<int> alphabet);
    static void mutate(std::vector<Individual> &pop, float mutationRate, std::vector<int> alphabet);
};

class UnivariateCrossover : public Variation {
public:
    UnivariateCrossover();
    std::pair<Individual, Individual> crossover(Individual &ind1, Individual &ind2) override;
    void display() override;
    std::string id() override;
};

class OnePointCrossover : public Variation {
public:
    OnePointCrossover();
    std::pair<Individual, Individual> crossover(Individual &ind1, Individual &ind2) override;
    void display() override;
    std::string id() override;
};

class TwoPointCrossover : public Variation {
public:
    TwoPointCrossover();
    std::pair<Individual, Individual> crossover(Individual &ind1, Individual &ind2) override;
    void display() override;
    std::string id() override;
};

class ThreePointCrossover : public Variation {
public:
    ThreePointCrossover();
    std::pair<Individual, Individual> crossover(Individual &ind1, Individual &ind2) override;
    void display() override;
    std::string id() override;
};

class FOSCrossover : public Variation {
public:
    FOSCrossover(FOS &fos);
    FOS *fosObject;
    std::pair<Individual, Individual> crossover(Individual &ind1, Individual &ind2) override;
    void display() override;
    std::string id() override;
};

class GOM_Variation : public Variation {
public:
    FitnessFunction *fitfunc;
    Individual *bestIndividual;
    bool forcedImprovement;
    std::vector<std::vector<int>> fos;
    FOS *fosObject;
    GOM_Variation(bool forcedInprovement = false);
    std::vector<Individual> variate(std::vector<Individual> &population) override;
    Individual gom(Individual &ind, std::vector<Individual> &population, int indIdx);
    bool gomWithEliteIndividual(Individual &o, Individual &b);
    void applyDonor(Individual &ind, Individual &parent, std::vector<int> &subset);
    void setFOSObject(FOS *f);
    void display() override;
    std::string id() override;
};

#endif /* Variation_hpp */
