//
//  GOMEA.hpp
//  GA
//
//  Created by Tom den Ottelander on 28/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef GOMEA_hpp
#define GOMEA_hpp

#include <stdio.h>
#include "GA.hpp"
#include "FOS.hpp"
#include "Variation.hpp"

class GOMEA : public GA {
public:
    FOS *fos;
    GOM_Variation gom_variation;

    GOMEA (FitnessFunction * fitfunc, FOS *fos, bool forcedImprovement);

    void initialize() override;

    void round() override;
    
    GA* clone() const override;
        
    std::string id() override;
};
    
#endif /* GOMEA_hpp */
