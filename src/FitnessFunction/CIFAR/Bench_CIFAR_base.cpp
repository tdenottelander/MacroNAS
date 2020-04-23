//
//  Bench_CIFAR_base.cpp
//  GA
//
//  Created by Tom den Ottelander on 16/12/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//
//  Contains a class Bench_CIFAR_base that derives from FitnessFunction.
//  This class is meant to process the results of the NAS Benchmark
//  that Arkadiy and Marco created.
//

#include "Bench_CIFAR_base.hpp"

using namespace std;
using namespace nlohmann;

extern string benchmarksDir;

Bench_CIFAR_base::Bench_CIFAR_base(int problemSize, bool allowIdentityLayers, bool genotypeChecking, ProblemType* problemType, int identity, int jsonAccuracyIndex, string folder) :
    FitnessFunction(vector<float>(1, getOptimum(folder, problemSize, allowIdentityLayers)), problemType),
    allowIdentityLayers(allowIdentityLayers),
    identityLayer(identity),
    jsonAccuracyIndex(jsonAccuracyIndex),
    folder(folder) {
        
        totalProblemLength = problemSize;
        if(genotypeChecking){
            setGenotypeChecking();
        }
        
//    setProblemType(allowIdentityLayers);
}

// Returns the fitness of an individual
vector<float> Bench_CIFAR_base::evaluate(Individual &ind){
    vector<float> fitness = query(ind.genotype);

    ind.fitness = fitness;
    
    evaluationProcedure(ind);
    return fitness;
}

// Returns the fitness of the architecture passed by its encoding by querying the benchmark
vector<float> Bench_CIFAR_base::query(vector<int> encoding){
    vector<float> result = getFitness(encoding);
    return result;
}

vector<float> Bench_CIFAR_base::getFitness(vector<int> encoding){
    //Transform encoding into string
    string layers;
    for (int i : encoding){
        //ignore identity layer
        if(i != identityLayer){
            layers += to_string(i);
        }
    }
    
    //Prepend "model_"
    //Append ".json"
    layers = benchmarksDir + folder + "/model_" + layers + ".json";
    
    //Load file
    ifstream ifs(layers);
    json rawdata = json::parse(ifs);
    
    //Retrieve the correct value (validation accuracy at epoch 120)
    float result = rawdata["val_acc_ensemble"].at(jsonAccuracyIndex);
    return vector<float>{result};
}

int Bench_CIFAR_base::getNumParams(vector<int> encoding){
    string layers;
    for (int i : encoding){
        if(i != identityLayer){
            layers += to_string(i);
        }
    }
    layers = benchmarksDir + folder + "/model_" + layers + ".json";
    ifstream ifs(layers);
    json rawdata = json::parse(ifs);
    return rawdata["number_of_parameters"];
}

void Bench_CIFAR_base::display(){
    cout << "Bench-CIFAR-base fitness function" << endl;
}

string Bench_CIFAR_base::id(){
    return ("Bench-CIFAR-base");
}

void Bench_CIFAR_base::setLength (int length){
    totalProblemLength = length;
}

void Bench_CIFAR_base::printArchitecture(vector<int> architecture){
    for (int i = 0; i < architecture.size(); i++){
        if(architecture[i] != 3){
            cout << architecture[i];
        } else {
            cout << "_";
        }
        if(i != architecture.size() - 1){
            cout << ",";
        }
    }
//    cout << endl;
}


vector<int> Bench_CIFAR_base::transform(vector<int> &genotype){
    return Individual::removeIdentities(genotype, identityLayer);
}

Bench_CIFAR_base::solution Bench_CIFAR_base::findBest (){
    vector<int> architecture (totalProblemLength, -1);
    solution statistics;
    statistics.fitness = -1.0;
    statistics.genotypes = {};
    statistics.optCount = 0;
    statistics.totalCount = 0;
    elitistArchive.clear();
    
    findBestRecursion(totalProblemLength, problemType->alphabet.size(), architecture, 0, statistics);
    
    cout << "Genotypes with optimal fitness: " << endl;
    for (vector<int> gen : statistics.genotypes){
        cout << Individual::toString(gen);
    }
    cout << "   accuracy: " << statistics.fitness;
//    cout << "   #params: " << getNumParams(opt.second) << endl;
    cout << endl;
    return statistics;
}

void Bench_CIFAR_base::findBestRecursion(int length, int alphabetSize, vector<int> &temp, int idx, solution &statistics){
    if (idx == length){
        vector<float> result = query (temp);
        Individual ind;
        ind.fitness = result;
        vector<int> gen (temp);
        ind.genotype = gen;
        updateElitistArchive(&ind);
        if (abs(result[0] - statistics.fitness) < 0.00001){
            statistics.genotypes.push_back(temp);
            statistics.optCount = statistics.optCount + 1;
        } else if (result[0] > statistics.fitness){
            statistics.fitness = result[0];
            statistics.genotypes = {temp};
            statistics.optCount = 1;
        }
        statistics.totalCount = statistics.totalCount + 1;
    } else {
        for (int i = 0; i < alphabetSize; i++){
            temp [idx] = i;
            findBestRecursion(length, alphabetSize, temp, idx+1, statistics);
        }
    }
}

pair<int, int> Bench_CIFAR_base::findAmountOfArchitecturesWithFitnessAboveThreshold(float threshold){
    int sum = 0;
    int total = pow(problemType->alphabet.size(), totalProblemLength);
    for (int i = 0; i < total; i++){
        vector<int> genotype = HashingFunctions::decode(i, totalProblemLength, problemType->alphabet.size());
        vector<float> result = query(genotype);
        if(result[0] >= threshold){
            cout << Individual::toString(genotype) << " f=" << result[0] << endl;
            sum++;
        }
    }
    cout << sum << "/" << total << " genotypes have fitness >= " << threshold << endl;
    return pair<int, int>(sum, total);
}

void Bench_CIFAR_base::doAnalysis(int minLayerSize, int maxLayerSize, string id){
    json results;
    json optima;
    json paretoFronts;
    
    for (int i = minLayerSize; i <= maxLayerSize; i++){
        totalProblemLength = i;
        solution statistics = findBest();
        vector<string> genotypes;
        for(vector<int> genotype : statistics.genotypes){
            string genotypeString;
            for(int gene : genotype){
                genotypeString += to_string(gene);
            }
            genotypes.push_back(genotypeString);
        }
        pair<float, vector<string>> opt(statistics.fitness, genotypes);
        optima[to_string(i)]["optimum"] = statistics.fitness;
        optima[to_string(i)]["genotypes"] = genotypes;
        optima[to_string(i)]["numGlobalOptima"] = statistics.optCount;
        optima[to_string(i)]["possibleGenotypes"] = statistics.totalCount;
        json paretoFront;
        json paretoFrontFitness;
        json paretoFrontGenotypes;
        sort(elitistArchive.begin(), elitistArchive.end(), [](const Individual lhs, const Individual rhs){
            if (lhs.fitness[0] > rhs.fitness[0]){
                return true;
            } else if (lhs.fitness[0] < rhs.fitness[0]){
                return false;
            } else {
                return lhs.fitness[1] > rhs.fitness[1];
            }
        });
        for (int j = 0; j < elitistArchive.size(); j++){
            paretoFrontFitness[j] = elitistArchive[j].fitness;
            paretoFrontGenotypes[j] = Utility::genotypeToString(elitistArchive[j].genotype);
        }
        paretoFront["fitness"] = paretoFrontFitness;
        paretoFront["genotypes"] = paretoFrontGenotypes;
        paretoFronts[to_string(i)] = paretoFront;
    }
    results["optima"] = optima;
    Utility::write(results.dump(), benchmarksDir, id + "_analysis.json");
    Utility::write(paretoFronts.dump(), benchmarksDir, id + "_paretofront.json");
}

float Bench_CIFAR_base::getOptimum(string folder, int layers, bool allowIdentityLayers){
    string filename = benchmarksDir + folder + "/analysis.json";
    ifstream ifs(filename);
    if(!ifs.good()){
        cout << "Cannot load optima. Consider first running the function \"doAnalysis\" first. Setting optimum to 100.0 for now." << endl;
        return 100.0;
    }
    cout << "Loading optima from " << filename << endl;
    json analysis = json::parse(ifs);
//    auto optGenotypes = analysis["optima"][to_string(layers)]["genotypes"];
//    vector<uvec> genotypes;
//    for (string gen : optGenotypes){
//        cout << gen << endl;
//        genotypes.push_back(Utility::stringToGenotype(gen));
//    }
//    int selectedGenotypeIdx = findMostDifferentGenotype(genotypes);
//    optimalGenotype = uvec(genotypes[selectedGenotypeIdx]);
//    cout << "optimal Genotype: " << Utility::genotypeToString(optimalGenotype);
    float result = analysis["optima"][to_string(layers)]["optimum"];
    return result;
}

vector<int> Bench_CIFAR_base::getOptimalGenotype(){
    string filename = benchmarksDir + folder + "/analysis.json";
    ifstream ifs(filename);
    if (!ifs.good()){
        cout << "Cannot load optima. Consider first running the function \"doAnalysis\" first. Setting optimal genotype to {0}." << endl;
        return {0};
    } else {
        json analysis = json::parse(ifs);
        auto optGenotypes = analysis["optima"][to_string(totalProblemLength)]["genotypes"];
        vector<vector<int>> genotypes;
        for (string gen : optGenotypes){
            cout << gen << endl;
            genotypes.push_back(Utility::stringToGenotype(gen));
        }
        int selectedGenotypeIdx = findMostDifferentGenotype(genotypes);
        vector<int> optGen (genotypes[selectedGenotypeIdx]);
        cout << "optimal Genotype: " << Utility::genotypeToString(optGen) << endl;
        return optGen;
    }
}

void Bench_CIFAR_base::setGenotypeChecking(){
    vector<int> optGen = getOptimalGenotype();
    FitnessFunction::setGenotypeChecking(optGen);
}

int Bench_CIFAR_base::findMostDifferentGenotype(vector<vector<int>> &genotypes){
    bool print = true;
    vector<vector<int>> distances;
    int n = genotypes.size();
    distances.reserve(n);
    for (int i = 0; i < n; i++){
        vector<int> distancesInner;
        distancesInner.reserve(n);
        distances.push_back(distancesInner);
    }
    
    int highestDist = -1;
    int mostDifferentGenotypeIdx = -1;
    for (int i = 0; i < genotypes.size(); i++){
        if (print) cout << "[ ";
        int total = 0;
        for (int j = 0; j < genotypes.size(); j++){
            int dist = Individual::hammingDistance(genotypes[i], genotypes[j]);
            distances[i].push_back(dist);
            if (print) cout << dist << " ";
            total += dist;
        }
        if (total > highestDist){
            highestDist = total;
            mostDifferentGenotypeIdx = i;
        }
        if (print) cout << "] (" << total << ")\n";
    }
    return mostDifferentGenotypeIdx;
}
