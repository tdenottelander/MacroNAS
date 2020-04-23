//
//  LocalSearchStochastic.hpp
//  GA
//
//  Created by Tom den Ottelander on 10/01/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef LocalSearchStochastic_hpp
#define LocalSearchStochastic_hpp

#include <stdio.h>
#include "LocalSearchBase.hpp"

class LocalSearchStochastic : public LocalSearchBase {
public:
    LocalSearchStochastic (FitnessFunction * fitFunc, Utility::Order localSearchOrder, float stochasticity);
    GA* clone() const override;
    std::string id() override;
};

#endif /* LocalSearchStochastic_hpp */
