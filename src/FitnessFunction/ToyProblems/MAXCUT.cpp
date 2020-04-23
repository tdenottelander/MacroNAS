//
//  MAXCUT.cpp
//  GA
//
//  Created by Tom den Ottelander on 02/03/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "MAXCUT.hpp"

using namespace std;

extern string benchmarksDir;

// ---------------------------- MAXCUT -------------------------------

MAXCUT::MAXCUT (int problemSize) : FitnessFunction(new BinaryProblemType()), maxcutmap(problemSize){
    totalProblemLength = problemSize;
    numObjectives = 2;
    
    setTrueParetoFront();
}


vector<float> MAXCUT::evaluate(Individual &ind) {
    
    int obj0 = 0;
    int obj1 = 0;
    
//    cout << ind.toString() << endl;
    
    for (int i = 0; i < ind.genotype.size(); i++){
        for (int j = i+1; j < ind.genotype.size(); j++){
            if (ind.genotype[i] != ind.genotype[j]){
                int obj0add = maxcutmap.get(0, i+1, j+1);
                int obj1add = maxcutmap.get(1, i+1, j+1);
//                cout << "i=" << i+1 << ",j=" << j+1 << ": obj0 add " << o0add << ", obj1 add " << o1add << endl;
                obj0 += obj0add;
                obj1 += obj1add;
            }
        }
    }

    vector<float> result {(float)obj0, (float)obj1};
    ind.fitness = result;

    evaluationProcedure(ind);

    return result;
}

FitnessFunction* MAXCUT::clone() const {
    return new MAXCUT(static_cast<const MAXCUT&>(*this));
}

bool MAXCUT::entireParetoFrontFound() {
    if (elitistArchive.size() == optimalParetoFrontSize){
        for (Individual &ind : elitistArchive){
            if((ind.fitness[0] + ind.fitness[1]) != totalProblemLength){
                return false;
            }
        }
        return true;
    }
    return false;
}

void MAXCUT::setTrueParetoFront(){
    trueParetoFront.clear();
    
    string filename = benchmarksDir + "maxcut/maxcut_pareto_front_" + to_string(totalProblemLength) + ".txt";
    ifstream ifs(filename);
    if(!ifs.good()){
        cout << "ERROR: Cannot find " << filename << endl;
        exit(-1);
    }
    bool firstline = true;
    while(ifs){
        string line;
        if (!getline(ifs, line)) break;
        
        if(firstline){
            optimalParetoFrontSize = stoi(line);
            trueParetoFront.reserve(optimalParetoFrontSize);
//            cout << "opt pareto front size: " << optimalParetoFrontSize << endl;
            firstline = false;
            continue;
        }
        
        istringstream ss(line);
        
        string obj0, obj1;
        getline(ss, obj0, '\t');
        getline(ss, obj1, '\t');
//        cout << obj0 << ", " << obj1 << endl;
        trueParetoFront.push_back(vector<float>{stof(obj0), stof(obj1)});
    }
}


// ---------------------------- Maxcutmap -------------------------------

Maxcutmap::Maxcutmap (int problemsize){
    map0 = parseProblemInstance(getFilename(problemsize, 0));
    map1 = parseProblemInstance(getFilename(problemsize, 1));
}

unordered_map<Maxcutmap::pair, int, Maxcutmap::pair_hash> Maxcutmap::parseProblemInstance(string filename){
    unordered_map<Maxcutmap::pair, int, Maxcutmap::pair_hash> map;
    ifstream ifs(filename);
    if(!ifs.good()){
        cout << "ERROR: Cannot find " << filename << endl;
        exit(-1);
    }
    bool firstline = true;
    while(ifs){
        string line;
        if (!getline(ifs, line)) break;
        
        if(firstline){
            firstline = false;
            continue;
        }
        
        istringstream ss(line);
        
        string v1, v2, edgeValue;
        getline(ss, v1, ' ');
        getline(ss, v2, ' ');
        getline(ss, edgeValue, ' ');
        map.insert({{stoi(v1), stoi(v2)}, stoi(edgeValue)});
    }
    
//    for (auto& x: map) {
//        std::cout << x.first.first << "," << x.first.second << ": " << x.second << '\n';
//    }
    
    return map;
}

int Maxcutmap::get(int obj, int v1, int v2){
    if (obj == 0) {
        return get(map0, v1, v2);
    } else if (obj == 1) {
        return get(map1, v1, v2);
    } else {
        return -1;
    }
}

int Maxcutmap::get(unordered_map<Maxcutmap::pair, int, Maxcutmap::pair_hash> &map, int v1, int v2){
    if(map.find({v1, v2}) == map.end()){
        return 0;
    } else {
        return map.at({v1, v2});
    }
}

string Maxcutmap::getFilename(int problemsize, int objective){
    return (benchmarksDir + "maxcut/maxcut_instance_" + to_string(problemsize) + "_" + to_string(objective) + ".txt");
}

