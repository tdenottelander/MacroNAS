# Code by Arkadiy Dushatskiy (github.com/ArkadiyD)

import random
import torch

class RandomHorizontalFlip(object):
    """Horizontally flip the given PIL Image randomly with a given probability.

    Args:
        p (float): probability of the image being flipped. Default value is 0.5
    """

    def __init__(self, p=0.5):
        self.p = p

    def __call__(self, img):
        """
        Args:
            img (PIL Image): Image to be flipped.

        Returns:
            PIL Image: Randomly flipped image.
        """
        #print (img.shape)
        #print (img)
        #print (torch.flip(img, dims=[2]))
        
        if random.random() < self.p:
            return torch.flip(img, dims=[2])
        return img

class RandomCrop(object):
    """Horizontally flip the given PIL Image randomly with a given probability.

    Args:
        p (float): probability of the image being flipped. Default value is 0.5
    """

    def __init__(self, p=0.5, size=32, padding=4):
        self.p = p
        self.pad = torch.nn.ConstantPad2d(padding, 0)
        self.size = size
        self.padding_size = padding

    def __call__(self, img):
        """
        Args:
            img (PIL Image): Image to be flipped.

        Returns:
            PIL Image: Randomly flipped image.
        """
        X, Y = img.shape[1], img.shape[2]
        #print (img.shape)
        if random.random() < self.p:
            img = self.pad(img)
            #print (img)
            x, y = random.randint(0, 2*self.padding_size), random.randint(0, 2*self.padding_size)
            img = img[:, x:x+self.size, y:y+self.size]
            #print(x,y,img)

        return img
