# MacroNAS

The source code is used for the paper:
> Den Ottelander T., Dushatskiy A., Virgolin M., and Bosman P. A. N.: Local Search is a Remarkably Strong Baseline for Neural Architecture Search. arXiv:2004.08996 (2020) https://arxiv.org/abs/2004.08996

## In this repository
#### source code for reproducing the experiments discussed in the paper
- Framework for searching on macro-level NAS search spaces:
  - cached NAS benchmark datasets on CIFAR-10 and CIFAR-100
  - real-time NAS with an enlarged search space on CIFAR-100
- 4 multi-objective search algorithms: MO-GOMEA, NSGA-II, LS and RS
- IMS framework for any population-based search algorithm


#### Additional source code
- 4 single-objective search algorithms: SimpleGA, GOMEA, LS and RS
- 2 single-objective toy problems: LeadingOnes, OneMax
- 3 multi-objective toy problems: ZeroMaxOneMax, TrapInverseTrap, LeadingOnesTrailingZeroes
- Various different cross-over operators for GAs
- Various (fixed) linkage models for (MO-)GOMEA

## Dependencies
- json by nlohmann: https://github.com/nlohmann/json


## How to use
0. ```git clone```
1. Download benchmark datasets from https://github.com/ArkadiyD/MacroNASBenchmark and place ```benchmark_cifar10_dataset.json``` and ```benchmark_cifar100_dataset.json``` (422MB each) inside the folder ```benchmarks```
2. To be able to run largescale experiments,<sup>1</sup> run ```pip3 install --user -r NAS_largescale/requirements.txt```
3. Make the project
4. Execute to run experiments by specifying the path to the executable obtained in the previous step:
  - ```bash run_bench_cifar10.sh [path_to_executable]```
  - ```bash run_bench_cifar100.sh [path_to_executable]```
  - ```bash run_largescale_cifar100.sh [path_to_executable]``` (modify contents in the file to run other algorithms besides MO-GOMEA)
5. (To see what other options for experiments are available, execute ```[path_to_executable] -?```)

<sup>1</sup> Note that when reproducing results on the large-scale search space, this requires integration of the C++ project with python3. Also, training and evaluating networks requires CUDA. Both parts might require specific solutions for your own machine, so detailed instructions for this are not included.
