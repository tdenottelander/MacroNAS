//
//  Utility.hpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#ifndef Utility_hpp
#define Utility_hpp

#include <stdio.h>
#include <vector>
#include <chrono>
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <random>

extern std::mt19937 rng;
extern std::uniform_real_distribution<float> dist;


namespace Utility{
    
    // Ordered array utility functions
    enum class Order {RANDOM, ASCENDING, DESCENDING};
    std::string orderToID (Order order);
    std::string orderToString (Order order);
    std::vector<int> getOrderedArray (int n, Order order);
    std::vector<int> getRandomlyPermutedArrayV2 (int n);
    std::vector<int> getAscendingArray (int n);
    std::vector<int> getDescendingArray (int n);
    
    // Random utility functions
    double getRand();
    int getRand(int start, int end);
    int getRand(std::vector<int> &vec);
    int getConditionalBit(int counter0, int counter1, int max);
    
    // Time & Date utility functions
    long millis();
    std::string getDateString();
    
    // Write utility functions
    void write(std::string content, std::string dir, std::string filename);
    void writeRawData(std::string content, std::string dir, std::string suffix);
    void writeRawData(std::string content);
    void writeRawData(std::string content, std::string filename);
    void writeJSON (nlohmann::json content, std::string filename);
    
    // Read utility functions
    nlohmann::json readJSON(std::string filename);
    void read(std::string filename);
    
    // String utility functions
    std::string genotypeToString(std::vector<int> &genotype);
    std::vector<int> stringToGenotype(std::string &genotype);
    std::string vecOfFloatsToString (std::vector<float> vec, std::string separator);
    std::string wrapWithBrackets (std::string str);
    std::string padFrontWith0(std::string target, int length);
    std::string removeTrailingZeros(std::string target);
    std::string padWithSpacesAfter(std::string target, int length);
    
    // Calculation utility functions
    float getAverage(std::vector<float> &vec);
    float getAverage(std::vector<int> &vec);
    float EuclideanDistanceSquared(std::vector<float> vecA, std::vector<float> vecB);
    float EuclideanDistance(std::vector<float> vecA, std::vector<float> vecB);
    bool isLogPoint(int value, int scaling=1);
    bool isLinearPoint(int value, int interval);
}

#endif /* Utility_hpp */
