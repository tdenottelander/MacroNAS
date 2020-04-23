//
//  LocalSearch.hpp
//  GA
//
//  Created by Tom den Ottelander on 10/01/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef LocalSearch_hpp
#define LocalSearch_hpp

#include <stdio.h>
#include "LocalSearchBase.hpp"

class LocalSearch : public LocalSearchBase {
public:
    LocalSearch (FitnessFunction * fitFunc, Utility::Order localSearchOrder);
    GA* clone() const override;
    std::string id() override;
};

#endif /* LocalSearch_hpp */

