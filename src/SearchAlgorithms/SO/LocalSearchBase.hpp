//
//  LocalSearchBase.hpp
//  GA
//
//  Created by Tom den Ottelander on 07/01/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef LocalSearchBase_hpp
#define LocalSearchBase_hpp

#include <stdio.h>
#include "GA.hpp"
#include "Utility.hpp"

class LocalSearchBase : public GA {
protected:
    LocalSearchBase (FitnessFunction * fitFunc, Utility::Order localSearchOrder, float stochasticity, bool loop = false);
    float stochasticity;
    bool loop;
public:
    Utility::Order localSearchOrder;
    void round() override;
    void doLocalSearch(Individual &ind, FitnessFunction * fitfunc, int maxRandomFlips = -1);
};

#endif /* LocalSearchBase_hpp */

