# Code by Arkadiy Dushatskiy (github.com/ArkadiyD)

import math
import torch.nn as nn
import torchvision.transforms as transforms
import augmentations
import numpy as np
from cells import *

class MultipleChoiceBlock(nn.Module):
    def __init__(self, layers = []):
        super(MultipleChoiceBlock, self).__init__()
        self.layers = nn.ModuleList(layers)

    def forward(self, x, propagate):
        
        output = 0
            
        for i in range(len(self.layers)):

            if propagate[i] == 0:
                continue

            output += self.layers[i](x)
            
        return output

def make_layers(first_conv_out_channels, model_encoding, normal_cells, downsampling_cells, put_downsampling):
    
    channels = first_conv_out_channels
    blocks = []
    for i in range(len(model_encoding)):        
        ind = model_encoding[i]
        if i in put_downsampling:            
            blocks.append(MultipleChoiceBlock([cell(channels) for cell in downsampling_cells]))
            channels *= 2
        else:
            blocks.append(MultipleChoiceBlock([cell(channels) for cell in normal_cells]))

    return blocks, channels

class NAS_NET(nn.Module):
    def __init__(self, num_classes, model_encoding, normal_cells, downsampling_cells, put_downsampling, first_conv_out_channels):
        super(NAS_NET, self).__init__()

        self.num_options_normal_cells = len(normal_cells)
        self.num_options_downsampling_cells = len(downsampling_cells)
        self.put_downsampling = put_downsampling
        self.model_encoding = model_encoding
        print ('encoding', self.model_encoding)
        self.in_conv = ConvBNReLU(3, first_conv_out_channels, kernel_size=3, stride=1)
        
        blocks, out_channels = make_layers(first_conv_out_channels, model_encoding, normal_cells, downsampling_cells, put_downsampling)
        self.blocks_list = nn.ModuleList(blocks)

        self.features_mixing = ConvBNReLU(out_channels, 1280, kernel_size=1, stride=1)

        self.out1 = nn.AdaptiveAvgPool2d((1, 1))
        self.out2 = nn.Linear(1280, num_classes)

    def forward(self, x):

        x = self.in_conv(x)
        #print (x.shape)
        for i, block in enumerate(self.blocks_list):

            if i in self.put_downsampling:
                propagate = [0 for i in range(self.num_options_downsampling_cells)]
            else:
                propagate = [0 for i in range(self.num_options_normal_cells)]
            
            if self.model_encoding[i] == -1: #train random paths
                if i in self.put_downsampling:
                    choice = np.random.randint(self.num_options_downsampling_cells)
                else:
                    choice = np.random.randint(self.num_options_normal_cells)
                propagate[choice] = 1
            else:
                propagate[self.model_encoding[i]] = 1

            #print (i, propagate, choice)

            x = block(x, propagate)

        #print (x.shape)
            
        x = self.features_mixing(x)
        #print (x.shape)

        x = self.out1(x)
        x = x.view(x.shape[0],-1)
        x = self.out2(x)        
        return x
