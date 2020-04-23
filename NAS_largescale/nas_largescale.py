# Code by Arkadiy Dushatskiy (github.com/ArkadiyD)

import os
import sys
import pickle
import itertools
import utils
import time
from collections import Counter

from sklearn.model_selection import StratifiedShuffleSplit
import numpy as np
from torch.utils.data import Dataset
import torchvision
import torchvision.transforms as transforms
import torch
import torch.nn as nn

from cells import *
import model
import nas_evaluate
import cifar

NORMAL_CELL_NAMES = ['I','1','2','3','4']
DOWNSAMPLING_CELL_NAMES = ['P','A','B','C','D']

N_CELLS = 17
DOWNSAMPLE_LAYERS = [4,9,14] #positions of downsampling layers

NORMAL_CELLS = [
            Identity,
            InvertedResidual_3_3,
            InvertedResidual_5_3,
            InvertedResidual_3_6,
            InvertedResidual_5_6,
        ]

DOWNSAMPLING_CELLS = [
            Conv_MaxPool,
            InvertedResidual_3_3_down,
            InvertedResidual_5_3_down,
            InvertedResidual_3_6_down,
            InvertedResidual_5_6_down,
        ]

DIR = 'cifar100'
DATASET = 'CIFAR100'
DATA_PATH = 'datasets'

SEED = 42
NUM_WORKERS = 4 #number of CPUs to use
BATCH_SIZE = 128
NUM_SAMPLES_TRAIN = 40000
NUM_SAMPLES_VAL = 10000
NUM_SAMPLES_TEST = 10000

FIRST_CONV_OUT_CHANNELS = 32 #first fixed conv layer, changes number of channels from 3 to 32

args_oneshot_model = {
'BATCH_SIZE': 128,
'EPOCHS': 500,
'LR_INIT': 0.1,
'LR_START_CYCLE': 400,
'CYCLE_PERIOD': 30
}

#we are finetuning the model from the one-shot architecture for 20 epochs with cosine-annealed learning rate (10x smaller than training supermodel)
#1 epoch takes 10-40 seconds depending on model (on average about 25 sec)
#the reported accuracies are from the last epoch
args_single_model = {
'BATCH_SIZE': 256,
'EPOCHS': 20,
'LR_INIT': 0.01,
'LR_START_CYCLE': 20,
'CYCLE_PERIOD': 0
}
############################################################################################################

def init(dir_name):
    global DIR, data_loaders

    DIR = dir_name
    print('Preparing directory %s' % DIR)
    os.makedirs(DIR, exist_ok=True)
    os.makedirs(DIR+'_results', exist_ok=True)

    print('Loading dataset %s from %s' % (DATASET, DATA_PATH))
    ds = getattr(torchvision.datasets, DATASET)
    path = os.path.join(DATA_PATH, DATASET.lower())

    if DATASET == 'CIFAR10':
        transforms_dict = cifar.get_cifar10_transforms()
    elif DATASET == 'CIFAR100':
        transforms_dict = cifar.get_cifar100_transforms()    
    else:
        print ('Dataset %s not implemented' % DATASET)
        exit(1)

    train_set = ds(path, train=True, download=True, transform=transforms_dict['train'])
    val_set = ds(path, train=True, download=True, transform=transforms_dict['val'])
    test_set = ds(path, train=False, download=True, transform=transforms_dict['test'])

    if not os.path.exists(os.path.join(DIR, 'indices.pkl')):
        #train/val split
        stratified_targets_split = StratifiedShuffleSplit(n_splits=1, test_size=0.2, random_state=42)
        for train_indices, val_indices in stratified_targets_split.split(train_set.targets, train_set.targets):
            print("TRAIN:", len(train_indices))
            print("VAL:", len(val_indices))
        
        #distilling train_for_bn split indicies
        stratified_targets_split = StratifiedShuffleSplit(n_splits=1, test_size=0.25, random_state=42)    
        train_targets = np.array(train_set.targets)[np.array(train_indices)]
        for _, train_for_bn_indices in stratified_targets_split.split(train_targets, train_targets):
            train_for_bn_indices = train_indices[train_for_bn_indices]
            print("TRAIN FOR BN:", len(train_for_bn_indices))
        
        indices = {'train':train_indices, 'val':val_indices, 'train_for_bn':train_for_bn_indices}
    
        pickle.dump(indices, open(os.path.join(DIR, 'indices.pkl'), 'wb'))

    else:
        indices = pickle.load(open(os.path.join(DIR, 'indices.pkl'), 'rb'))
        print(indices.keys())
        train_indices, val_indices, train_for_bn_indices = indices['train'], indices['val'], indices['train_for_bn']

    train_targets = np.array(train_set.targets)[train_indices]
    val_targets = np.array(train_set.targets)[val_indices]
    train_for_bn_targets = np.array(train_set.targets)[train_for_bn_indices]
    
    print (Counter(train_targets))
    print (Counter(val_targets))
    print (Counter(train_for_bn_targets))
    assert(np.intersect1d(train_indices, val_indices).shape[0]==0)
    assert(np.intersect1d(train_for_bn_indices, val_indices).shape[0]==0)
    assert(np.intersect1d(train_indices, train_for_bn_indices).shape[0]==train_for_bn_indices.shape[0])
    
    
    train_sampler = torch.utils.data.SubsetRandomSampler(train_indices)
    val_sampler = torch.utils.data.SubsetRandomSampler(val_indices)
    train_to_bn_sampler = torch.utils.data.SubsetRandomSampler(train_for_bn_indices)


    data_loaders = {
        'val': torch.utils.data.DataLoader(
            val_set, #without augmentations
            batch_size=BATCH_SIZE, 
            sampler=val_sampler,
            #shuffle=False,
            num_workers=NUM_WORKERS,
            pin_memory=True
        ),

        'test': torch.utils.data.DataLoader(
            test_set,  #without augmentations
            batch_size=BATCH_SIZE,
            shuffle=False,
            num_workers=NUM_WORKERS,
            pin_memory=True),

        'train': torch.utils.data.DataLoader(
            train_set, #with_augmentations
            batch_size=BATCH_SIZE,
            sampler=train_sampler,
            #shuffle=True,
            num_workers=NUM_WORKERS,
            pin_memory=True
        ),

        'train_for_bn_recalc': torch.utils.data.DataLoader(
            val_set, #without augmentations
            batch_size=BATCH_SIZE,
            sampler=train_to_bn_sampler,
            #shuffle=False,
            num_workers=NUM_WORKERS,
            pin_memory=True)
    }

def train_one_shot_model():
    torch.cuda.empty_cache()

    solution = [-1 for l in range(N_CELLS)]
    model_description = 'oneshot_model'        
    print('m', model_description)
    
    if utils.check_model_exist(DIR, model_description):
        print('supermodel model exists')
        return

    net = model.NAS_NET(cifar.get_number_of_classes(DATASET), solution, NORMAL_CELLS, DOWNSAMPLING_CELLS, DOWNSAMPLE_LAYERS, FIRST_CONV_OUT_CHANNELS)
    net_ensemble = model.NAS_NET(cifar.get_number_of_classes(DATASET), solution, NORMAL_CELLS, DOWNSAMPLING_CELLS, DOWNSAMPLE_LAYERS, FIRST_CONV_OUT_CHANNELS)
    #print (net)
    training_args = {'seed': SEED,
                    'batch_size': args_oneshot_model['BATCH_SIZE'],
                    'lr_init': args_oneshot_model['LR_INIT'],
                    'lr_start_cycle': args_oneshot_model['LR_START_CYCLE'],
                    'cycle_period': args_oneshot_model['CYCLE_PERIOD'],
                    'epochs': args_oneshot_model['EPOCHS'],
                    'num_samples_train': NUM_SAMPLES_TRAIN,
                    'num_samples_val': NUM_SAMPLES_VAL,
                    'num_samples_test': NUM_SAMPLES_TEST,
                    'dir': DIR,
                    'img_size': cifar.get_size(DATASET),
                    'input_channels': cifar.get_number_of_input_channels(DATASET)                    
                    }

    try:
        result = nas_evaluate.run_evaluation(net, net_ensemble, data_loaders, training_args, save_model = 'supermodel')
        
        utils.save_result(result, DIR, model_description)
        
    except Exception as e:
        print(e)

def evaluate(solution):

    torch.cuda.empty_cache()

    #real model description means the actual set of layers omitting the identities
    model_description, real_model_description = [], []
    for l in range(N_CELLS):
        if l in DOWNSAMPLE_LAYERS:
            model_description.append(DOWNSAMPLING_CELL_NAMES[solution[l]])
            real_model_description.append(DOWNSAMPLING_CELL_NAMES[solution[l]])
        else:            
            model_description.append(NORMAL_CELL_NAMES[solution[l]])
            if NORMAL_CELL_NAMES[solution[l]] != 'I':
                real_model_description.append(NORMAL_CELL_NAMES[solution[l]])
    
    print('m', model_description)
    print('r', real_model_description)
    
    #check if the identical model already evaluated, if yes - just return results
    if utils.check_model_exist(DIR, real_model_description):
        print ('identical model exists')
        #utils.copy_solution(DIR, model_description, real_model_description)
        results = utils.load_json(DIR, real_model_description)
        return results['MMACs'], results['val_acc'][0], results['test_acc'][0]

    start = time.time()
    #create model
    net = model.NAS_NET(cifar.get_number_of_classes(DATASET), solution, NORMAL_CELLS, DOWNSAMPLING_CELLS, DOWNSAMPLE_LAYERS, FIRST_CONV_OUT_CHANNELS)
    #net_ensemble = model.NAS_NET(cifar.get_number_of_classes(DATASET), solution, NORMAL_CELLS, DOWNSAMPLING_CELLS, DOWNSAMPLE_LAYERS, FIRST_CONV_OUT_CHANNELS)
    print ('models creation', time.time() - start)

    training_args = {'seed': SEED,
                    'batch_size': args_single_model['BATCH_SIZE'],
                    'lr_init': args_single_model['LR_INIT'],
                    'lr_start_cycle': args_single_model['LR_START_CYCLE'],
                    'cycle_period': args_single_model['CYCLE_PERIOD'],
                    'epochs': args_single_model['EPOCHS'],
                    'num_samples_train': NUM_SAMPLES_TRAIN,
                    'num_samples_val': NUM_SAMPLES_VAL,
                    'num_samples_test': NUM_SAMPLES_TEST,
                    'dir': DIR,
                    'img_size': cifar.get_size(DATASET),
                    'input_channels': cifar.get_number_of_input_channels(DATASET)                    
                    }

    try:
        #do actual evaluation
        results = nas_evaluate.run_evaluation_single_model(net, data_loaders, training_args, load_model = 'supermodel_ensemble')
        print (results)
        #save jsons with results (both initial model encoding and the actual model (without identities))
        utils.save_result(results, DIR, model_description)
        utils.save_result(results, DIR, real_model_description)
        return results['MMACs'], results['val_acc'][0], results['test_acc'][0]

    except Exception as e:
        print(e)

if __name__ == '__main__':
    init(DIR)
    train_one_shot_model()
    
    #examples of evaluating solutions
   
    solution1 = [2,2,1,2,3,4,1,2,3,4,1,2,3,0,1,2,3]
    solution2 = [2,2,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3]
    evaluate([4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4])
    evaluate([1,3,4,1,0,4,3,2,4,1,3,2,1,4,1,1,1])
    evaluate(solution1)
    evaluate(solution2)

