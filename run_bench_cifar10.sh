#!/bin/bash
if [ $# -eq 0 ]; then
  echo "Please provide the path to the executable as parameter like so: bash run_bench_cifar10.sh [path_to_executable]"
  exit 1
fi
path_to_executable=$1
path_to_project=src/../

netuniqevals=100000
fit="cifar10 14 2" # optimize on 14 cells of the architecture, on 2 objectives
convergence=entire_pareto
repetitions=30
populationInitializationMode=1 # 0 for all random, 1 to include the all-identity solution at the start
printAtEveryEval=0 # Set to 1 for printing every evaluation
saveLogFilesOnEveryUpdate=0 # Set to 1 for saving log files on every evaluation
outfilesuffix="_cifar10"

seed=10

optimizer=MO-GOMEA
fos=learned
IMS=1
$path_to_executable -P $path_to_project -S $seed -I $IMS -n $netuniqevals -f $fit -c $convergence -F $fos -o $optimizer -r $repetitions -M $populationInitializationMode -q $printAtEveryEval -x $saveLogFilesOnEveryUpdate -O $optimizer$suffix

optimizer=NSGA-II
variation=2p
popsize=100
$path_to_executable -P $path_to_project -S $seed -p $popsize -n $netuniqevals -f $fit -c $convergence -v $variation -o $optimizer -r $repetitions -M $populationInitializationMode -q $printAtEveryEval -x $saveLogFilesOnEveryUpdate -O $optimizer$suffix

optimizer=MO-RS
$path_to_executable -P $path_to_project -S $seed -n $netuniqevals -f $fit -c $convergence -o $optimizer -r $repetitions -M $populationInitializationMode -q $printAtEveryEval -x $saveLogFilesOnEveryUpdate -O $optimizer$suffix

optimizer=MO-LS
$path_to_executable -P $path_to_project -S $seed -n $netuniqevals -f $fit -c $convergence -o $optimizer -r $repetitions -M $populationInitializationMode -q $printAtEveryEval -x $saveLogFilesOnEveryUpdate -O $optimizer$suffix
