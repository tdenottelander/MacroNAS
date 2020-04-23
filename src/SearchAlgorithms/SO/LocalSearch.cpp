//
//  LocalSearch.cpp
//  GA
//
//  Created by Tom den Ottelander on 10/01/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "LocalSearch.hpp"

using namespace std;

LocalSearch::LocalSearch (FitnessFunction * fitFunc, Utility::Order localSearchOrder) : LocalSearchBase(fitFunc, localSearchOrder, 0) {}

GA* LocalSearch::clone() const {
    return new LocalSearch(static_cast<const LocalSearch&>(*this));
}

string LocalSearch::id() {
    return ("LocalSearch-" + Utility::orderToID(localSearchOrder));
}
