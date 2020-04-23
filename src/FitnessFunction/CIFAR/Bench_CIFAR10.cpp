//
//  Bench_CIFAR10.cpp
//  GA
//
//  Created by Tom den Ottelander on 09/03/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "Bench_CIFAR10.hpp"

using namespace std;
using namespace nlohmann;

extern string benchmarksDir;

Bench_CIFAR10::Bench_CIFAR10(int problemSize, bool genotypeChecking, bool MO) : Bench_CIFAR_base(problemSize, false, genotypeChecking, getProblemType(), -1, 2, "cifar10"), MO(MO)
{
    string id = "benchmark_cifar10";
    if(lookupTable.empty()){
        string filename = benchmarksDir + id + "_dataset.json";
        
        cout << "Reading in CIFAR-10 results from " + filename + ". This may take a while.... ";
        ifstream ifs(filename);
        if(!ifs.good()){
            cout << "ERROR: cannot read results." << endl;
        } else {
            lookupTable = json::parse(ifs);
            cout << "Done loading CIFAR-10 results" << endl;
        }
    }
    
    if(MO){
        setOptimum(vector<float>{-1, -1});
        setNumObjectives(2);
        
        string filename = benchmarksDir + id + "_paretofront.json";
        cout << "Reading in CIFAR-10 pareto results from " + filename + ".... ";
        ifstream ifs(filename);
        if(!ifs.good()){
            doAnalysis(0, 14, id);
            ifstream ifs(filename);
        }
        if (!ifs.good()){
            cout << "ERROR: cannot read pareto file." << endl;
            exit(0);
        } else {
            json output = json::parse(ifs);
            json paretoInformation = output[to_string(problemSize)]["fitness"];
            for (int i = 0; i < paretoInformation.size(); i++){
                trueParetoFront.push_back(paretoInformation[i]);
            }
            cout << "Done loading CIFAR-10 Pareto results" << endl;
        }
        optimalParetoFrontSize = trueParetoFront.size();
    }
    networkLibrary.type = SolutionLibrary::Type::BENCHMARK;
    storeNetworkUniqueEvaluations = true;
}


vector<float> Bench_CIFAR10::evaluate(Individual &ind){
    vector<float> fitness;
    bool totalNetworkUniqueEvaluationsUpdate = false;
    
    if (networkLibrary.contains(ind.genotype)){
        fitness = networkLibrary.get(ind.genotype);
    } else {
        fitness = getFitness(ind.genotype);
        networkLibrary.put(ind.genotype, fitness);
        totalNetworkUniqueEvaluations++;
        totalNetworkUniqueEvaluationsUpdate = true;
    }
    
    ind.fitness = fitness;
    
    evaluationProcedure(ind);
    
    if(totalNetworkUniqueEvaluationsUpdate && log(totalNetworkUniqueEvaluations)){
        logNetworkUniqueEvaluations();
    }
    
    return fitness;
}

vector<float> Bench_CIFAR10::getFitness (vector<int> encoding){
    string layers;
    for (int i = 0; i < 14; i++){
        // Appends identity layers to the back of the architecture when we are dealing with problem lengths < 14.
        if(i >= totalProblemLength){
            layers += "I";
        } else {
            if(encoding[i] == 0){
                layers += "I";
            } else {
                layers += to_string(encoding[i]);
            }
        }
    }
    
    float acc = lookupTable[layers]["val_acc"];
    if (MO){
        float mmacs = lookupTable[layers]["MMACs"];
        float normalizedmmacs = 1.0f - ((mmacs - minMMACs) / (maxMMACs - minMMACs));
        return vector<float> {acc * 0.01f, normalizedmmacs};
    } else {
        return vector<float>{acc * 0.01f};
    }
}

void Bench_CIFAR10::display(){
    cout << "Bench CIFAR-10 fitness function" << endl;
}

string Bench_CIFAR10::id(){
    string res = "CIFAR-10-";
    res += (numObjectives == 1 ? "SO" : "MO");
    return res;
}

FitnessFunction* Bench_CIFAR10::clone() const {
    return new Bench_CIFAR10(static_cast<const Bench_CIFAR10&>(*this));
}

ProblemType* Bench_CIFAR10::getProblemType (){
    return new AlphabetProblemType({0,1,2});
}
