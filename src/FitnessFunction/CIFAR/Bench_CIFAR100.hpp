//
//  Bench_CIFAR100.hpp
//  GA
//
//  Created by Tom den Ottelander on 29/01/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef Bench_CIFAR100_hpp
#define Bench_CIFAR100_hpp

#include <stdio.h>
#include "Bench_CIFAR_base.hpp"
#include <algorithm>

class Bench_CIFAR100 : public Bench_CIFAR_base {
public:
    Bench_CIFAR100(int problemSize, bool genotypeChecking, bool MO = false);
    bool MO = false;

    // For normalizing the MMACs objective
    float minMMACs = 21.54;
    float maxMMACs = 239.51;
    
    std::vector<float> evaluate (Individual &ind) override;
    std::vector<float> getFitness (std::vector<int> encoding) override;
    
    void display() override;
    std::string id() override;
    FitnessFunction* clone() const override;
    
    ProblemType* getProblemType();
};

#endif /* Bench_CIFAR100_hpp */
