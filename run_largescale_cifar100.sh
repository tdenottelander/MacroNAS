#!/bin/bash
if [ $# -eq 0 ]; then
  echo "Please provide the path to the executable as parameter like so: bash run_largescale_cifar100.sh [path_to_executable]"
  exit 1
fi
path_to_executable=$1
path_to_project=src/../  #Will point to the current working directory

networkUniqueEvals=2500
dataset="cifar100"
fit="cifar-largescale 17 2" # optimize on 17 cells of the architecture, on 2 objectives
repetitions=1
populationInitializationMode=1 #1 includes all-identity solution, 0 is all random
printAtEveryEval=1 # Set to 1 to print every evaluation
saveLogFilesOnEveryUpdate=1 # Set to 1 to save log files on every evaluation
gpu=0
seed=10

##### MO-GOMEA
fos="learned"
optimizer="MO-GOMEA"
IMS=1
CUDA_VISIBLE_DEVICES=$gpu $path_to_executable -S $seed -P $path_to_project -d $dataset -f $fit -F $fos -o $optimizer -r $repetitions -I $IMS -M $populationInitializationMode -q $printAtEveryEval -x $saveLogFilesOnEveryUpdate

##### NSGA-II
# variation="2p"
# optimizer="NSGA-II"
# popsize=100
# CUDA_VISIBLE_DEVICES=$gpu $path_to_executable -S $seed -P $path_to_project -d $dataset -f $fit -v $variation -o $optimizer -r $repetitions -p $popsize -M $populationInitializationMode -q $printAtEveryEval -x $saveLogFilesOnEveryUpdate

##### RS
# optimizer="RS"
# CUDA_VISIBLE_DEVICES=$gpu $path_to_executable -S $seed -P $path_to_project -d $dataset -f $fit -o $optimizer -r $repetitions -M $populationInitializationMode -q $printAtEveryEval -x $saveLogFilesOnEveryUpdate

##### LS
# optimizer="LS"
# CUDA_VISIBLE_DEVICES=$gpu $path_to_executable -S $seed -P $path_to_project -d $dataset -f $fit -o $optimizer -r $repetitions -M $populationInitializationMode -q $printAtEveryEval -x $saveLogFilesOnEveryUpdate
