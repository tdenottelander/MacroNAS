//
//  FitnessFunction.hpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef FitnessFunction_hpp
#define FitnessFunction_hpp

#include <stdio.h>
#include <nlohmann/json.hpp>
#include <tuple>
#include <numeric>
#include "Individual.hpp"
#include "ProblemType.hpp"
#include "SolutionLibrary.hpp"

class FitnessFunction {
public:
    
    enum class ConvergenceCriteria {OPTIMAL_FITNESS, EPSILON_FITNESS, GENOTYPE, ENTIRE_PARETO, EPSILON_PARETO_DISTANCE, NONE};
    
    ConvergenceCriteria convergenceCriteria = ConvergenceCriteria::NONE;
    Individual bestIndividual;
    std::vector<Individual> elitistArchive;
    std::vector<float> optimum;
    int optimalParetoFrontSize;
    std::vector<std::vector<float>> trueParetoFront;
    bool optimumFound;
    int maxEvaluations;
    int maxUniqueEvaluations;
    int maxNetworkUniqueEvaluations;
    ProblemType *problemType;
    int totalProblemLength;
    int numObjectives = 1;
    bool checkForGenotype = false;
    std::vector<int> optimalGenotype;
    float epsilon = 0.0f;
    bool done = false;
    
    int totalEvaluations;
    int totalUniqueEvaluations;
    int totalNetworkUniqueEvaluations;
    bool storeNetworkUniqueEvaluations = false;
    SolutionLibrary uniqueSolutions;
    
    nlohmann::json elitistArchiveToJSON();
    
    FitnessFunction(std::vector<float> optimum, ProblemType *problemType);
    FitnessFunction(ProblemType *problemType);
    
    virtual void clear();
    
    virtual std::vector<float> evaluate(Individual &ind) = 0;
    void evaluationProcedure(Individual &ind);
    bool log(int evals);
    bool isDone();
    bool isMO();
    virtual void display();
    virtual std::string id();
    void setProblemType(ProblemType* problemType);
    
    void checkIfBestFound(Individual &ind);
    bool updateElitistArchive(Individual* ind);
    bool updateElitistArchive(std::vector<Individual> &front);
    bool updateElitistArchive(std::vector<Individual*> &front);
    virtual bool entireParetoFrontFound ();
    std::pair<float, float> calculateDistanceParetoToApproximation ();
    int paretoPointsFound();
    void setGenotypeChecking(std::vector<int> genotype);
    
    int getTotalAmountOfEvaluations();
    bool maxEvaluationsExceeded();
    bool maxUniqueEvaluationsExceeded();
    bool maxNetworkUniqueEvaluationsExceeded();
    
    virtual FitnessFunction* clone() const = 0;
    
    virtual void setLength (int length);
    virtual void setNumObjectives (int numObjectives);
    virtual void setOptimum (std::vector<float> optimum);
    virtual void setOptimum (float optimum);
    
    virtual std::vector<int> transform(std::vector<int> &genotype);
    
    void draw2DVisualization(std::vector<Individual> &population, int maxX, int maxY);
    void drawElitistArchive();
    
    void saveElitistArchiveToJSON();
    int storeElitistArchiveCount = 0;
    
    void printElitistArchive(bool fullArchive = true);
    
    void logNetworkUniqueEvaluations();
    SolutionLibrary networkLibrary;
};

class OneMax : public FitnessFunction {
public:
    OneMax (int length);
    OneMax ();
    std::vector<float> evaluate(Individual &ind) override;
    void display() override;
    std::string id() override;
    ProblemType* getProblemType();
    FitnessFunction* clone() const override;
};


class LeadingOnes : public FitnessFunction {
public:
    LeadingOnes (int length);
    LeadingOnes ();
    std::vector<float> evaluate(Individual &ind) override;
    void display() override;
    std::string id() override;
    ProblemType* getProblemType();
    FitnessFunction* clone() const override;
};

class NonBinaryMax : public FitnessFunction {
public:
    NonBinaryMax ();
    std::vector<float> evaluate(Individual &ind) override;
    void display() override;
    std::string id() override;
    ProblemType* getProblemType();
    FitnessFunction* clone() const override;
    void setLength (int length) override;
};

#endif /* FitnessFunction_hpp */
