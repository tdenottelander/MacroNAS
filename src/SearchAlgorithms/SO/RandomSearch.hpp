//
//  RandomSearch.hpp
//  GA
//
//  Created by Tom den Ottelander on 16/12/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef RandomSearch_hpp
#define RandomSearch_hpp

#include <stdio.h>
#include "GA.hpp"

class RandomSearch : public GA {
public:
    RandomSearch (FitnessFunction * fitFunc);
    void round() override;
    GA* clone() const override;
    std::string id() override;
};

#endif /* RandomSearch_hpp */
