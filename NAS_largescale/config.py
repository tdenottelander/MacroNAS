# Code by Arkadiy Dushatskiy (github.com/ArkadiyD)

import argparse
import nas_test
import torch
import asyncio

parser = argparse.ArgumentParser(description='nas settings')

parser.add_argument('--dir', type=str, default=None, required=True, help='training directory (default: None)')
parser.add_argument('--dataset', type=str, default='CIFAR10', help='dataset name (default: CIFAR10)')
parser.add_argument('--data_path', type=str, default=None, required=True, metavar='PATH', help='path to datasets location (default: None)')
parser.add_argument('--model', type=str, default='FFNET', metavar='MODEL', help='model name (default: None)')
parser.add_argument('--batch_size', type=int, default=256, metavar='N', help='input batch size (default: 256)')
parser.add_argument('--epochs', type=int, default=100, metavar='N', help='number of epochs to train (default: 200)')
parser.add_argument('--lr_init', type=float, default=0.1, metavar='LR', help='initial learning rate (default: 0.01)')
parser.add_argument('--seed', type=int, default=1, metavar='seed', help='random seed (default: 1)')
parser.add_argument('--num_workers', type=int, default=8, metavar='num_workers', help='num of workers (default: 16)')
parser.add_argument('--half_precision', type=int, default=0, metavar='half_precision', help='whether to use half precision (default: True)')
parser.add_argument('--lr_start_cycle', type=int, default=40, metavar='lr_start_cycle', help='epoch when cycling learning rate starts (default: 50)')
parser.add_argument('--cycle_period', type=int, default=10, metavar='cycle_period', help='period of cycling learnign rate (default: 10)')
parser.add_argument('--num_samples_train', type=int, default=40000, metavar='num_samples_train', help='samples in train part (default: 40000')
parser.add_argument('--num_samples_val', type=int, default=10000, metavar='num_samples_val', help='samples in validation part (default: 10000')
parser.add_argument('--num_samples_test', type=int, default=10000, metavar='num_samples_test', help='samples in test part (default: 10000')

parser.add_argument('--first_net_id', type=int, default=0, required=False, metavar='first_net_id', help='first net architecture id to test one')
parser.add_argument('--last_net_id', type=int, default=5000, required=False, metavar='last_net_id', help='last net architecture id to test one')
args = parser.parse_args()
