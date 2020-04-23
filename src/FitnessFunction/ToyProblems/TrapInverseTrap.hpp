//
//  TrapInverseTrap.hpp
//  GA
//
//  Created by Tom den Ottelander on 06/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef TrapInverseTrap_hpp
#define TrapInverseTrap_hpp

#include <stdio.h>
#include "FitnessFunction.hpp"

class TrapInverseTrap : public FitnessFunction {
public:
    int blocks;
    int blocksize = 5;
    TrapInverseTrap (int length);
    std::vector<float> evaluate(Individual &ind) override;
    float subfunc (Individual &ind, int startIdx, int endIdx, int objective);
    void display() override;
    std::string id() override;
    FitnessFunction* clone() const override;
    bool entireParetoFrontFound() override;
};

#endif /* TrapInverseTrap_hpp */

