//
//  MAXCUT.hpp
//  GA
//
//  Created by Tom den Ottelander on 02/03/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef MAXCUT_hpp
#define MAXCUT_hpp

#include <stdio.h>
#include "FitnessFunction.hpp"
#include <unordered_map>
#include <sstream>


class Maxcutmap {
public:
    typedef std::pair<int, int> pair;
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator() (const std::pair<T1, T2> &pair) const {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };
    
    std::unordered_map<pair, int, pair_hash> map0;
    std::unordered_map<pair, int, pair_hash> map1;
    
    Maxcutmap (int problemsize);
    int get(int obj, int v1, int v2);
    int get(std::unordered_map<Maxcutmap::pair, int, Maxcutmap::pair_hash> &map, int v1, int v2);
    std::string getFilename(int problemsize, int objective);
    std::unordered_map<pair, int, pair_hash> parseProblemInstance(std::string filename);
};

class MAXCUT : public FitnessFunction {
public:
    MAXCUT(int problemSize);
    
    Maxcutmap maxcutmap;
    
    std::vector<float> evaluate (Individual &ind) override;
    
    FitnessFunction* clone() const override;
//    void setOptimum(std::vector<float> optimum) override;
    void setTrueParetoFront();
    bool entireParetoFrontFound() override;
};


#endif /* MAXCUT_hpp */
