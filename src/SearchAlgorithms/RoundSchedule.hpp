//
//  RoundSchedule.hpp
//  GA
//
//  Created by Tom den Ottelander on 22/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef RoundSchedule_hpp
#define RoundSchedule_hpp

#include <stdio.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include "GA.hpp"
#include "NSGA_II.hpp"
#include "Selection.hpp"
#include "Variation.hpp"
#include "Utility.hpp"
#include "SolutionLibrary.hpp"

class RoundSchedule {
public:
    int maxRounds;
    int maxPopSizeLevel;
    int maxSeconds;
    int maxEvaluations;
    int maxUniqueEvaluations;
    int maxNetworkUniqueEvaluations;
    int interval;
    Selection* selection;
    Variation* variation;
    GA* ga;
    std::vector<GA*> gaList;
    std::vector<int> whichShouldRun;
    RoundSchedule(int maxRounds, int maxPopSizeLevel, int maxSeconds, int maxEvaluations, int maxUniqueEvaluations, int maxNetworkUniqueEvaluations, int interleavedRoundInterval);
    void initialize(GA *ga, int problemSize, bool nonIMSmode = false, int nonIMSpopsize = 1);
    void run();
    void terminateGAs(int n);
    bool MOterminationCondition(GA* gaCurrent, GA* gaPrev, float percentageRequired);
    int getAmountOfEvaluations();
    bool maxEvaluationsExceeded();
    bool maxUniqueEvaluationsExceeded();
    void writeOutputGenerationCSV(std::string filename);
};

#endif /* RoundSchedule_hpp */
