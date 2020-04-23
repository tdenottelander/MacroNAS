//
//  LocalSearchStochastic.cpp
//  GA
//
//  Created by Tom den Ottelander on 10/01/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "LocalSearchStochastic.hpp"

using namespace std;

LocalSearchStochastic::LocalSearchStochastic (FitnessFunction * fitFunc, Utility::Order localSearchOrder, float stochasticity) : LocalSearchBase(fitFunc, localSearchOrder, stochasticity) {}

GA* LocalSearchStochastic::clone() const {
    return new LocalSearchStochastic(static_cast<const LocalSearchStochastic&>(*this));
}

string LocalSearchStochastic::id() {
    return ("LocalSearchStoch-" + Utility::removeTrailingZeros(to_string(stochasticity)) + "-" + Utility::orderToID(localSearchOrder));
}
