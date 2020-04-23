//
//  RS.hpp
//  GA
//
//  Created by Tom den Ottelander on 26/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef RS_hpp
#define RS_hpp

#include <stdio.h>
#include "GA.hpp"

class RS : public GA {
public:
    RS (FitnessFunction * fitFunc);
    void round() override;
    GA* clone() const override;
    std::string id() override;
};

#endif /* RS_hpp */
