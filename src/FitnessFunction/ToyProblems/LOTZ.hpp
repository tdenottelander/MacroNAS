//
//  LOTZ.hpp
//  GA
//
//  Created by Tom den Ottelander on 05/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef LOTZ_hpp
#define LOTZ_hpp

#include <stdio.h>
#include "FitnessFunction.hpp"

class LOTZ : public FitnessFunction {
public:
    LOTZ(int problemSize);
    std::vector<float> evaluate (Individual &ind) override;
    
    FitnessFunction* clone() const override;
    
    void setOptimum(std::vector<float> optimum) override;
    bool entireParetoFrontFound() override;
    std::string id() override;
};

#endif /* LOTZ_hpp */
