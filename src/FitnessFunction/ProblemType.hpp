//
//  ProblemType.hpp
//  GA
//
//  Created by Tom den Ottelander on 06/12/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef ProblemType_hpp
#define ProblemType_hpp

#include <stdio.h>
#include <vector>
#include <string>

class ProblemType {
public:
    std::vector<int> alphabet;
    virtual std::string id();
};

class BinaryProblemType : public ProblemType {
public:
    BinaryProblemType ();
    std::string id() override;
};

class AlphabetProblemType : public ProblemType {
public:
    AlphabetProblemType (std::vector<int> alphabet);
    std::string id() override;
};

#endif /* ProblemType_hpp */
