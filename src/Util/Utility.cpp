//
//  Utility.cpp
//  GA
//
//  Created by Tom den Ottelander on 19/11/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "Utility.hpp"

using namespace std;
using namespace chrono;
using namespace nlohmann;

unsigned long long randcount = 0;

extern string dataDir;

/* ------------------------ Ordered array utility functions------------------------ */

// Returns an specified ordered array of length n   (containing values [0, 1, ..., n-2, n-1])
vector<int> Utility::getOrderedArray(int n, Order order){
    switch (order) {
        case Order::RANDOM:
            return getRandomlyPermutedArrayV2(n);
        case Order::ASCENDING:
            return getAscendingArray(n);
        case Order::DESCENDING:
            return getDescendingArray(n);
    }
}

string Utility::orderToID(Order order){
    switch (order) {
        case Order::RANDOM:
            return "rand";
        case Order::ASCENDING:
            return "asc";
        case Order::DESCENDING:
            return "desc";
    }
}

string Utility::orderToString(Order order){
    switch (order) {
        case Order::RANDOM:
            return "Random";
        case Order::ASCENDING:
            return "Ascending";
        case Order::DESCENDING:
            return "Descending";
    }
}

// Returns a randomly permuted array of length n   (containing values [0, 1, ..., n-2, n-1])
vector<int> Utility::getRandomlyPermutedArrayV2 (int n){
    vector<int> arr = Utility::getAscendingArray(n);
    
    vector<int> result (n, 0);
    for (int i = 0; i < n; i++){
        float rand = Utility::getRand();
        int idx = floor(rand * arr.size());
        result[i] = arr[idx];
        arr.erase(arr.begin()+idx);
    }
    
    return result;
}

// Returns an ascending array of length n   [0, 1, ..., n-2, n-1]
vector<int> Utility::getAscendingArray(int n){
    vector<int> arr (n, 0);
    for(int i = 0; i < n; i++)
        arr[i] = i;
    return arr;
}

// Returns a descending array of length n   [n-1, n-2, ..., 1, 0]
vector<int> Utility::getDescendingArray(int n){
    vector<int> arr (n, 0);
    for (int i = 0; i < n; i++)
        arr[i] = n - i - 1;
    return arr;
}


/* ------------------------ Random utility functions------------------------ */

// Returns a uniformly random double between 0.0 and 1.0
double Utility::getRand(){
    randcount++;
    return dist(rng);
}

// Returns a uniformly random integer between begin (including) and end (excluding)
int Utility::getRand(int begin, int end){
    return floor(begin + (end - begin) * Utility::getRand());
}

// Returns a random integer from an array of ints.
int Utility::getRand(std::vector<int> &vec){
    int idx = Utility::getRand(0, vec.size());
    return vec[idx];
}

int Utility::getConditionalBit(int counter0, int counter1, int max){
    int leftFor0 = (max / 2) - counter0;
    int leftFor1 = (max / 2) - counter1;
    int totalLeft = leftFor0 + leftFor1;
    
    float chanceOn0 = (float)leftFor0 / totalLeft;
    if (Utility::getRand() < chanceOn0){
        return 0;
    } else {
        return 1;
    }
}


/* ------------------------ Time & Date utility functions------------------------ */

long Utility::millis(){
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

string Utility::getDateString(){
    std::time_t t = std::time(0);   // get time now
    std::tm* now = std::localtime(&t);
    string result = to_string(now->tm_year - 100);
    result += padFrontWith0(to_string(now->tm_mon + 1), 2);
    result += padFrontWith0(to_string(now->tm_mday), 2);
    result += "_";
    result += padFrontWith0(to_string(now->tm_hour), 2);
    result += padFrontWith0(to_string(now->tm_min), 2);
    result += padFrontWith0(to_string(now->tm_sec), 2);
    result += "_";
    result += to_string(millis());
    return result;
}


/* ------------------------ Write utility functions------------------------ */

void Utility::write(string content, string dir, string filename){
    ofstream file;
    file.open (dir + filename);
    file << content;
    file.close();
}

void Utility::writeRawData(string content, string dir, string suffix){
    if(suffix != "") suffix = "_" + suffix;
    write(content, dir, getDateString() + "_rawdata" + suffix + ".json");
}

void Utility::writeRawData(string content){
    writeRawData(content, dataDir, "");
}

void Utility::writeRawData(string content, string filename){
    ofstream file;
    file.open (filename);
    file << content;
    file.close();
}

void Utility::writeJSON(json content, string filename){
    ofstream file;
    file.open(dataDir + filename);
    file << content.dump();
    file.close();
}


/* ------------------------ Read utility functions------------------------ */

json Utility::readJSON(string filename){
    ifstream file;
    file.open(filename);
    if(!file){
        cerr << "Unable to open file " + filename;
        exit(1);   // call system to stop
    }
    json result = json::parse(file);
    return result;
}

void Utility::read(string filename){
    ifstream file;
    file.open(filename);
    if(!file){
        cerr << "Unable to open file " + filename;
        exit(1);   // call system to stop
    }
    string s;
    while (file >> s) {
        cout << s;
    }
    cout << endl;
    file.close();
}


/* ------------------------ String utility functions------------------------ */

// Converts a genotype in the form of vector<int> to a string of concatenated integers
string Utility::genotypeToString(vector<int> &genotype){
    string result = "";
    for(int i : genotype)
        result += to_string(i);
    return result;
}

// Converts a string of integers to a genotype in the form of vector<int>
vector<int> Utility::stringToGenotype (string &genotype){
    int n = genotype.size();
    vector<int> result(n, 0);
    for (int i = 0; i < n; i++){
        result[i] = stoi(genotype.substr(i,1));
    }
    return result;
}

// Converts a vector of floats to a string, seperating the floats by the specified [separator]
string Utility::vecOfFloatsToString (vector<float> vec, string separator){
    string result = "";
    for (int i = 0; i < vec.size(); i++){
        result += to_string(vec[i]);
        if (i < vec.size()-1){
            result += separator;
        }
    }
    return result;
}

// Returns the target string with brackets around it
string Utility::wrapWithBrackets (string str){
    return ("[" + str + "]");
}

// Returns the target string with zeros prepended, such that it is of length [length]
string Utility:: padFrontWith0(string target, int length){
    int curLength = target.size();
    for (int i = 0; i < (length - curLength); i++) target = "0" + target;
    return target;
}

// Returns the target string with the last zeros removed
string Utility::removeTrailingZeros(string target){
    int lastNonZero = target.size();
    for (int i = target.size() - 1; i >= 0; i--){
        if(target.at(i) != '0'){
            lastNonZero = i;
            break;
        }
    }
    return target.substr(0, lastNonZero + 1);
}

// Returns the target string with spaces appended after it, such that it is of length [length]
string Utility::padWithSpacesAfter(string target, int length){
    int n = target.size();
    for (int i = 0; i < length - n; i++)
        target = target + " ";
    return target;
}


/* ------------------------ Calculation utility functions------------------------ */

float Utility::getAverage(vector<float> &vec){
    return accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
}

float Utility::getAverage(vector<int> &vec){
    return accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
}

float Utility::EuclideanDistanceSquared(vector<float> vecA, vector<float> vecB){
    float result = 0.0;
    for (int i = 0; i < vecA.size(); i++){
        result += pow(vecA[i] - vecB[i], 2);
    }
    return result;
}

float Utility::EuclideanDistance(vector<float> vecA, vector<float> vecB){
    return sqrt(EuclideanDistanceSquared(vecA, vecB));
}

// Returns true if evaluations is a point on a log10 scale.
// (So return true if evaluations = 1, 2, 3, 10, 20, 30, 100, 200, 1000, 2000, ...
//   and return false if evaluations = 11, 101, 1001, 10001, 10002, 10003, ...)
bool Utility::isLogPoint(int value, int scaling){
    for (int i = 1; i < 15; i++){
        if (value <= pow(10, i)){
            int divisor = (int)pow(10, i-1);
            if (i > 1){
                divisor = (int)(divisor/scaling);
            }
            return value % divisor == 0;
        }
    }
    return false;
}

// Returns true if evaluations is a point on the linear scale with intervals of size [interval].
// (So return true if interval = 10 and evaluations = 0, 10, 20, 30, ..., 100, 110, 120, ..., 100000, 100010, ...
//   and return false if interval = 10 and evaluations = 1, 2, ..., 11, 12, ..., 100001, 100002, ...
bool Utility::isLinearPoint(int value, int interval){
    return value % interval == 0;
}
