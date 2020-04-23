//
//  LS.hpp
//  GA
//
//  Created by Tom den Ottelander on 07/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef LS_hpp
#define LS_hpp

#include <stdio.h>
#include <queue>
#include "GA.hpp"
#include "Utility.hpp"

class LS : public GA {
public:
    enum class NewScalarization {RANDOM, SCALARIZATIONSPACE, OBJECTIVESPACE};
    LS(FitnessFunction * fitfunc);
    LS(FitnessFunction * fitfunc, Utility::Order order, bool loop, NewScalarization newScalarization);
    Utility::Order LS_order;
    bool loop;
    NewScalarization newScalarization;
    
    std::queue<float> scalarizationTargets;
    std::vector<std::pair<std::pair<float, float>, std::vector<float>>> LS_archive;
    
    void round() override;
    void performLocalSearch(Individual &ind, std::vector<float> scalarization);
    bool dominates(Individual &indThis, Individual &indOther, std::vector<float> scalarization);
    float scalarizeFitness(Individual &ind, std::vector<float> scalarization);
    float getNewScalarizationTarget();
    
    void updateLSArchive(std::pair<float, float> scalarization, std::vector<float> objectiveValues);
    bool equalObjectiveValues(std::vector<float> &o1, std::vector<float> &o2);
    
    GA* clone() const override;
    std::string id() override;
    std::string NewScalarizationToString(NewScalarization newScalarization);
};

#endif /* LS_hpp */
