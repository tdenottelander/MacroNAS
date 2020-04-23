//
//  ProblemType.cpp
//  GA
//
//  Created by Tom den Ottelander on 06/12/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "ProblemType.hpp"

using namespace std;

std::string ProblemType::id() {
    return "Basic ProblemType";
}

BinaryProblemType::BinaryProblemType (){
    vector<int> vect {0,1};
    ProblemType::alphabet = vect;
}

std::string BinaryProblemType::id() {
    return "Binary";
}

AlphabetProblemType::AlphabetProblemType (vector<int> alphabet){
    ProblemType::alphabet = alphabet;
}

string AlphabetProblemType::id(){
    string result = "Alphabet[";
    for (int i = 0; i < alphabet.size(); i++){
        result += to_string(alphabet[i]);
        if (i < alphabet.size()-1){
            result += ",";
        }
    }
    result += "]";
    return result;
}
