//
//  ZeroMaxOneMax.hpp
//  GA
//
//  Created by Tom den Ottelander on 04/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef ZeroMaxOneMax_hpp
#define ZeroMaxOneMax_hpp

#include <stdio.h>
#include "FitnessFunction.hpp"

class ZeroMaxOneMax : public FitnessFunction {
public:
    ZeroMaxOneMax(int problemSize);
    std::vector<float> evaluate (Individual &ind) override;
    
    FitnessFunction* clone() const override;
    void setOptimum(std::vector<float> optimum) override;
    bool entireParetoFrontFound() override;
    std::string id() override;
};

#endif /* ZeroMaxOneMax_hpp */
