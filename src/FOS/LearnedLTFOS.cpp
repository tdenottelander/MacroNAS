//
//  LearnedLTFOS.cpp
//  GA
//
//  Created by Tom den Ottelander on 09/12/2019.
//  Copyright © 2019 Tom den Ottelander. All rights reserved.
//

#include "LearnedLTFOS.hpp"

using namespace std;


/* ------------------------ Learned Linkage Tree FOS ------------------------ */

LearnedLT_FOS::LearnedLT_FOS(ProblemType *problemType) : problemType(problemType){
    reinitializeOnNewRound = true;
}

vector<vector<int>> LearnedLT_FOS::getFOS(vector<Individual> &population){
    vector<vector<int>> fos = GenerateLinkageTreeFOS(population);
    return fos;
}

vector<vector<int>> LearnedLT_FOS::getFOS (int genotypeLength){
    cout << "Error, not implemented" << endl;
    exit(0);
}

string LearnedLT_FOS::id() { return "LearnedLT"; }
string LearnedLT_FOS::toString(){ return "Learned Linkage Tree FOS"; }


// Original function implementation by Marco Virgolin (https://github.com/marcovirgolin/GP-GOMEA/blob/master/GOMEA/GOMEAFOS.cpp).
// Adapted by Tom den Ottelander.
vector<vector<int>> LearnedLT_FOS::GenerateLinkageTreeFOS(const std::vector<Individual>& population) {
    
    size_t problemLength = population[0].genotype.size();

    vector<vector<int>> FOS;

    size_t pop_size = population.size();

    // phase 1: estimate similarity matrix
    vector<vector < double_t >> mi_matrix(problemLength, vector<double_t>(problemLength, 0.0));

    // build frequency table for symbol pairs
    size_t alphabetSize = problemType->alphabet.size();
    vector<vector<int>> frequencies(alphabetSize, vector<int>(alphabetSize, 0));
    uint val_i, val_j;
    
    uint encode_number = alphabetSize;

    // measure frequencies of pairs & compute joint entropy
    for (uint i = 0; i < problemLength; i++) {
        for (uint j = i + 1; j < problemLength; j++) {
            for (uint p = 0; p < pop_size; p++) {
                val_i = population[p].genotype[i];
                val_j = population[p].genotype[j];

                frequencies[val_i][val_j] += 1.0;
            }

            double_t freq;
            for (uint k = 0; k < encode_number; k++) {
                for (uint l = 0; l < encode_number; l++) {
                    freq = frequencies[k][l];
//                    if(pop_size > 4){
//                        cout << "i: " << i << " j: " << j << " k: " << k << " l: " << l << " freq: " << freq << endl;
//                    }
                    if (freq > 0.0) {
                        freq = freq / pop_size;
                        mi_matrix[i][j] += -freq * log2(freq);
                        frequencies[k][l] = 0.0; // reset the freq;
                    }
                }
            }
            mi_matrix[j][i] = mi_matrix[i][j];
        }

        for (uint p = 0; p < pop_size; p++) {
            val_i = population[p].genotype[i];
            frequencies[val_i][val_i] += 1.0;
        }

        double_t freq;
        for (uint k = 0; k < encode_number; k++) {
            for (uint l = 0; l < encode_number; l++) {
                freq = frequencies[k][l];
//                if(pop_size > 4){
//                    cout << "i: " << i << " j: " << i << " k: " << k << " l: " << l << " freq: " << freq << endl;
//                }
                if (freq > 0) {
                    freq = freq / pop_size;
                    mi_matrix[i][i] += -freq * log2(freq);
                    frequencies[k][l] = 0.0; // reset the freq;
                }
            }
        }
    }

    // transform entropy into mutual information
    for (size_t i = 0; i < problemLength; i++) {
        for (size_t j = i + 1; j < problemLength; j++) {
            mi_matrix[i][j] = mi_matrix[i][i] + mi_matrix[j][j] - mi_matrix[i][j];
            mi_matrix[j][i] = mi_matrix[i][j];
        }
    }
    
//    for (size_t i = 0; i < problemLength; i++) {
//        for (size_t j = 0; j < problemLength; j++) {
//            printf("%.2f  ", mi_matrix[i][j]);
//        }
//        cout << endl;
//    }
    
    // assemble the Linkage Tree with UPGMA
    FOS = BuildLinkageTreeFromSimilarityMatrix(problemLength, mi_matrix);

    FOS.pop_back(); // remove the root of the Linkage Tree (otherwise entire solution can be swapped during GOM)

    return FOS;
}

// Code by Peter Bosman.
// Adapted by Tom den Ottelander.
vector<vector<int>> LearnedLT_FOS::BuildLinkageTreeFromSimilarityMatrix(size_t number_of_nodes, vector<vector<double_t>> &sim_matrix) {
    
    vector<vector<int>> FOS;
    
    vector<int> random_order = Utility::getRandomlyPermutedArrayV2(number_of_nodes);
    
    vector<vector < int >> mpm(number_of_nodes, vector<int>(1));
    vector<int> mpm_number_of_indices(number_of_nodes);
    int mpm_length = number_of_nodes;
    
    for (size_t i = 0; i < number_of_nodes; i++) {
        mpm[i][0] = random_order[i];
        mpm_number_of_indices[i] = 1;
    }
    
    /* Initialize LT to the initial MPM */
    FOS.resize(number_of_nodes + number_of_nodes - 1);
    size_t FOSs_index = 0;
    for (int i = 0; i < mpm_length; i++) {
        vector<int> vec (mpm[i].size());
        for (int j = 0; j < mpm[i].size(); j++){
            vec[j] = mpm[i][j];
        }
        FOS[i] = vec;
        FOSs_index++;
    }
    
    /* Initialize similarity matrix */
    vector<vector < double_t >> S_matrix(number_of_nodes, vector<double_t>(number_of_nodes));
    for (int i = 0; i < mpm_length; i++)
        for (int j = 0; j < mpm_length; j++)
            S_matrix[i][j] = sim_matrix[mpm[i][0]][mpm[j][0]];
    for (int i = 0; i < mpm_length; i++)
        S_matrix[i][i] = 0;
    
    vector<vector < int >> mpm_new;
    vector<int> NN_chain(number_of_nodes + 2, 0);
    int NN_chain_length = 0;
    short done = 0;
    
    while (!done) {
        if (NN_chain_length == 0) {
            NN_chain[NN_chain_length] = (int) Utility::getRand() * mpm_length;
            NN_chain_length++;
        }
        
        while (NN_chain_length < 3) {
            NN_chain[NN_chain_length] = DetermineNearestNeighbour(NN_chain[NN_chain_length - 1], S_matrix, mpm_number_of_indices, mpm_length);
            NN_chain_length++;
        }
        
        while (NN_chain[NN_chain_length - 3] != NN_chain[NN_chain_length - 1]) {
            NN_chain[NN_chain_length] = DetermineNearestNeighbour(NN_chain[NN_chain_length - 1], S_matrix, mpm_number_of_indices, mpm_length);
            if (((S_matrix[NN_chain[NN_chain_length - 1]][NN_chain[NN_chain_length]] == S_matrix[NN_chain[NN_chain_length - 1]][NN_chain[NN_chain_length - 2]])) && (NN_chain[NN_chain_length] != NN_chain[NN_chain_length - 2]))
                NN_chain[NN_chain_length] = NN_chain[NN_chain_length - 2];
            NN_chain_length++;
            if (NN_chain_length > number_of_nodes)
                break;
        }
        int r0 = NN_chain[NN_chain_length - 2];
        int r1 = NN_chain[NN_chain_length - 1];
        int rswap;
        if (r0 > r1) {
            rswap = r0;
            r0 = r1;
            r1 = rswap;
        }
        NN_chain_length -= 3;
        
        if (r1 < mpm_length) { /* This test is required for exceptional cases in which the nearest-neighbor ordering has changed within the chain while merging within that chain */
            vector<int> indices(mpm_number_of_indices[r0] + mpm_number_of_indices[r1]);
            //indices.resize((mpm_number_of_indices[r0] + mpm_number_of_indices[r1]));
            //indices.clear();
            
            int i = 0;
            for (int j = 0; j < mpm_number_of_indices[r0]; j++) {
                indices[i] = mpm[r0][j];
                i++;
            }
            for (int j = 0; j < mpm_number_of_indices[r1]; j++) {
                indices[i] = mpm[r1][j];
                i++;
            }
            
            FOS[FOSs_index] = indices;
            FOSs_index++;
            
            double_t mul0 = ((double_t) mpm_number_of_indices[r0]) / ((double_t) mpm_number_of_indices[r0] + mpm_number_of_indices[r1]);
            double_t mul1 = ((double_t) mpm_number_of_indices[r1]) / ((double_t) mpm_number_of_indices[r0] + mpm_number_of_indices[r1]);
            for (i = 0; i < mpm_length; i++) {
                if ((i != r0) && (i != r1)) {
                    S_matrix[i][r0] = mul0 * S_matrix[i][r0] + mul1 * S_matrix[i][r1];
                    S_matrix[r0][i] = S_matrix[i][r0];
                }
            }
            
            mpm_new = vector<vector < int >> (mpm_length - 1);
            vector<int> mpm_new_number_of_indices(mpm_length - 1);
            int mpm_new_length = mpm_length - 1;
            for (i = 0; i < mpm_new_length; i++) {
                mpm_new[i] = mpm[i];
                mpm_new_number_of_indices[i] = mpm_number_of_indices[i];
            }
            
            mpm_new[r0] = vector<int>(indices.begin(), indices.end());
            
            mpm_new_number_of_indices[r0] = mpm_number_of_indices[r0] + mpm_number_of_indices[r1];
            if (r1 < mpm_length - 1) {
                mpm_new[r1] = mpm[mpm_length - 1];
                mpm_new_number_of_indices[r1] = mpm_number_of_indices[mpm_length - 1];
                
                for (i = 0; i < r1; i++) {
                    S_matrix[i][r1] = S_matrix[i][mpm_length - 1];
                    S_matrix[r1][i] = S_matrix[i][r1];
                }
                
                for (int j = r1 + 1; j < mpm_new_length; j++) {
                    S_matrix[r1][j] = S_matrix[j][mpm_length - 1];
                    S_matrix[j][r1] = S_matrix[r1][j];
                }
            }
            
            for (i = 0; i < NN_chain_length; i++) {
                if (NN_chain[i] == mpm_length - 1) {
                    NN_chain[i] = r1;
                    break;
                }
            }
            
            mpm = mpm_new;
            mpm_number_of_indices = mpm_new_number_of_indices;
            mpm_length = mpm_new_length;
            
            if (mpm_length == 1)
                done = 1;
        }
    }
    
    return FOS;
}

int LearnedLT_FOS::DetermineNearestNeighbour(int index, vector<vector<double_t>> &S_matrix, vector<int> & mpm_number_of_indices, int mpm_length) {
    int i, result;
    
    result = 0;
    if (result == index)
        result++;
    for (i = 1; i < mpm_length; i++) {
        if (((S_matrix[index][i] > S_matrix[index][result]) || ((S_matrix[index][i] == S_matrix[index][result]) && (mpm_number_of_indices[i] < mpm_number_of_indices[result]))) && (i != index))
            result = i;
    }
    
    return ( result);
}

vector<vector<int>> LearnedLT_FOS::transformLinkageTreeFOS(vector<vector<size_t>> FOS){
    vector<vector<int>> result;
    result.reserve(FOS.size());
    for (vector<size_t> vec : FOS){
        vector<int> newVec (vec.size(), 0);
        for (size_t i = 0; i < vec.size(); i++){
            newVec[i] = vec[i];
        }
        result.push_back(newVec);
    }
    return result;
}
