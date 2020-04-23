//
//  CIFAR_Largescale.cpp
//  GA
//
//  Created by Tom den Ottelander on 28/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#include "CIFAR_Largescale.hpp"

using namespace std;

extern string path_JSON_Progress;
extern nlohmann::json JSON_Progress;
extern string dataset;
extern string projectDir;

CIFAR_Largescale::CIFAR_Largescale (int problemSize, int numberOfObjectives) : FitnessFunction(getProblemType()) {
    networkLibrary.type = SolutionLibrary::Type::LARGE_SCALE;
    numObjectives = numberOfObjectives;
    totalProblemLength = problemSize;
    storeNetworkUniqueEvaluations = true;
    pythonInit();
}

vector<float> CIFAR_Largescale::evaluate(Individual &ind){
    
    vector<float> fitness;
    bool totalNetworkUniqueEvaluationsUpdate = false;
    
    if (networkLibrary.contains(ind.genotype)){
        fitness = networkLibrary.get(ind.genotype);
    } else {
        fitness = pyEvaluate(ind);
        networkLibrary.put(ind.genotype, fitness);
        totalNetworkUniqueEvaluations++;
        totalNetworkUniqueEvaluationsUpdate = true;
    }
    
    ind.fitness = fitness;
    
    evaluationProcedure(ind);

    if(totalNetworkUniqueEvaluationsUpdate && log(totalNetworkUniqueEvaluations)){
        logNetworkUniqueEvaluations();
    }
    logAndSaveProgress(ind);
    
//    cout << ind.toString() << " evals: " << totalEvaluations << " uniqEvals: " << totalUniqueEvaluations << " netUniqEvals: " << totalNetworkUniqueEvaluations << endl;
//    cout << "Lib size: " << networkLibrary.library.size() << endl;
    
    return fitness;
}

vector<float> CIFAR_Largescale::pyEvaluate(Individual &ind){
    // Build a python array from the individual's genotype
    int N = ind.genotype.size();
    PyObject* py_args = PyList_New(N);
    for (int i = 0; i < N; i++){
        PyObject* python_int = Py_BuildValue("i", ind.genotype[i]);
        PyList_SetItem(py_args, i, python_int);
    }

    // Create a tuple from the python array
    PyObject* py_tuple = PyTuple_Pack(1, py_args);

    // Check if the reference to the evaluation function is still correct
    if(!py_evaluationFunction){
        PyErr_Print();
        exit(-1);
    }

    // Evaluate the result
    PyObject* result = PyObject_CallObject(py_evaluationFunction, py_tuple);
    if(!result){
        PyErr_Print();
        exit(-1);
    }

    // Unpack the result
    int mmacs = PyLong_AsLong(PyTuple_GetItem(result,0));
    float val_acc = PyFloat_AsDouble(PyTuple_GetItem(result, 1));
    float test_acc = PyFloat_AsDouble(PyTuple_GetItem(result, 2));

    // Return the normalized fitness values
    if (numObjectives == 2){
        float normalizedmmacs = 1.0f - ((mmacs - minMMACs) / (maxMMACs - minMMACs));
        return vector<float> {val_acc * 0.01f, normalizedmmacs};
    } else {
        return vector<float>{val_acc * 0.01f};
    }
}

void CIFAR_Largescale::display(){
    cout << "CIFAR_largescale fitness function" << endl;
}

string CIFAR_Largescale::id(){
    return ("CIFAR-largescale-" + to_string(numObjectives) + "-" + dataset);
}

ProblemType* CIFAR_Largescale::getProblemType(){
    vector<int> alphabet = {0,1,2,3,4};
    return new AlphabetProblemType(alphabet);
}

FitnessFunction* CIFAR_Largescale::clone() const {
    return new CIFAR_Largescale(static_cast<const CIFAR_Largescale&>(*this));
}

void CIFAR_Largescale::setLength (int length){
    totalProblemLength = length;
}

void CIFAR_Largescale::pythonInit(){
    
    // Init python connection
    Py_Initialize();
    
    //Path to folder containing python script
    PyObject* sysPath = PySys_GetObject((char*)"path");
    string nas_online_path = projectDir + "NAS_largescale/";
    PyList_Append(sysPath, PyUnicode_FromString(nas_online_path.c_str()));

    // Import python module/script
    module = PyImport_ImportModule("nas_largescale");
    if(module == NULL){
        PyErr_Print();
        printf("ERROR importing module 'nas_largescale'\n");
        exit(-1);
    }

    // Load the data
    PyObject* py_initFunc = PyObject_GetAttrString(module, "init");
    if(!py_initFunc){
        PyErr_Print();
        exit(-1);
    }
    
    const char *dirname = dataset.c_str();
    PyObject* py_dirname = PyUnicode_DecodeFSDefault(dirname);
    PyObject* py_tuple = PyTuple_Pack(1, py_dirname);
    PyObject_CallObject(py_initFunc, py_tuple);

    // Set a reference to the evaluation function
    py_evaluationFunction = PyObject_GetAttrString(module, "evaluate");
    if(!py_evaluationFunction){
        PyErr_Print();
        exit(-1);
    }
}

void CIFAR_Largescale::logAndSaveProgress(Individual &ind){
    JSON_Progress["total_evals"].push_back(totalEvaluations);
    JSON_Progress["unique_evals"].push_back(totalUniqueEvaluations);
    JSON_Progress["network_unique_evals"].push_back(totalNetworkUniqueEvaluations);
    JSON_Progress["evaluated_solution_genotype"].push_back(Utility::genotypeToString(ind.genotype));
    JSON_Progress["evaluated_solution_network"].push_back(networkLibrary.hash(ind.genotype));
    JSON_Progress["evaluated_solution_fitness"].push_back(ind.fitness);
    
    if (numObjectives == 1){
        JSON_Progress["best_solution_genotype"].push_back(Utility::genotypeToString(bestIndividual.genotype));
        JSON_Progress["best_solution_network"].push_back(networkLibrary.hash(bestIndividual.genotype));
        JSON_Progress["best_solution_fitness"].push_back(bestIndividual.fitness);
    } else {
        JSON_Progress["elitist_archive"].push_back(elitistArchiveToJSON());
    }
    
    Utility:: writeRawData(JSON_Progress.dump(), path_JSON_Progress);
}
