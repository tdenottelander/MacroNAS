//
//  CIFAR_Largescale.hpp
//  GA
//
//  Created by Tom den Ottelander on 28/02/2020.
//  Copyright © 2020 Tom den Ottelander. All rights reserved.
//

#ifndef CIFAR_Largescale_hpp
#define CIFAR_Largescale_hpp

#include <stdio.h>
#include <vector>
#include <Python.h>
#include "FitnessFunction.hpp"

static PyObject *module;
static PyObject *py_evaluationFunction;

class CIFAR_Largescale : public FitnessFunction {
public:
    // For normalizing the MMACs objective
    float minMMACs = 13.03;
    float maxMMACs = 255.86;

    CIFAR_Largescale(int problemSize, int numberOfObjectives);
    std::vector<float> evaluate(Individual &ind) override;
    std::vector<float> pyEvaluate(Individual &ind);
    void display() override;
    std::string id() override;
    ProblemType* getProblemType();
    FitnessFunction* clone() const override;
    void setLength (int length) override;
    
    void logAndSaveProgress(Individual &ind);
    
    static void pythonInit();
};

#endif /* CIFAR_Largescale_hpp */
