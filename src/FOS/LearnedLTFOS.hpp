//
//  LearnedLTFOS.hpp
//  GA
//
//  Created by Tom den Ottelander on 09/12/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef LearnedLTFOS_hpp
#define LearnedLTFOS_hpp

#include <stdio.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include "FOS.hpp"
#include "Utility.hpp"
#include "Individual.hpp"
#include "ProblemType.hpp"

class LearnedLT_FOS : public FOS {
public:
    ProblemType *problemType;
    LearnedLT_FOS(ProblemType *problemType);
    std::vector<std::vector<int>> getFOS (std::vector<Individual> &population) override;
    std::vector<std::vector<int>> getFOS (int genotypeLength) override;
    std::string id() override;
    std::string toString() override;
    std::vector<std::vector<int>> GenerateLinkageTreeFOS(const std::vector<Individual>& population);
    std::vector<std::vector<int>> BuildLinkageTreeFromSimilarityMatrix(size_t number_of_nodes, std::vector<std::vector<double_t>> &sim_matrix);
    int DetermineNearestNeighbour(int index, std::vector<std::vector<double_t>> &S_matrix, std::vector<int> & mpm_number_of_indices, int mpm_length);
    std::vector<std::vector<int>> transformLinkageTreeFOS(std::vector<std::vector<size_t>>);
};

#endif /* LearnedLTFOS_hpp */
