# Code by Arkadiy Dushatskiy (github.com/ArkadiyD)

from torch import nn

class Identity(nn.Module):
    def __init__(self, *args, **kwargs):
        super().__init__()
    def forward(self, x):
        return x

class ConvBNReLU(nn.Sequential):
    def __init__(self, in_planes, out_planes, kernel_size=3, stride=1, groups=1):
        padding = (kernel_size - 1) // 2
        super(ConvBNReLU, self).__init__(
            nn.Conv2d(in_planes, out_planes, kernel_size, stride, padding, groups=groups, bias=False),
            nn.BatchNorm2d(out_planes),
            nn.ReLU6(inplace=True)
        )

class InvertedResidual(nn.Module):
    def __init__(self, inp, oup, kernel_size, stride, expand_ratio, use_res_connect):
        super(InvertedResidual, self).__init__()
        self.stride = stride
        assert stride in [1, 2]

        hidden_dim = int(round(inp * expand_ratio))

        self.kernel_size = kernel_size
        self.use_res_connect = use_res_connect

        layers = []
        if expand_ratio != 1:
            # pw
            layers.append(ConvBNReLU(inp, hidden_dim, kernel_size=1))
            layers.extend([
            # dw
            ConvBNReLU(hidden_dim, hidden_dim, stride=stride, groups=hidden_dim, kernel_size=self.kernel_size),
            # pw-linear
            nn.Conv2d(hidden_dim, oup, 1, 1, 0, bias=False),
            nn.BatchNorm2d(oup),
        ])
        self.conv = nn.Sequential(*layers)

    def forward(self, x):
        if self.use_res_connect:
            return x + self.conv(x)
        else:
            return self.conv(x)

class InvertedResidual(nn.Module):
    def __init__(self, inp, oup, kernel_size, stride, expand_ratio, use_res_connect):
        super(InvertedResidual, self).__init__()
        self.stride = stride
        assert stride in [1, 2]

        hidden_dim = int(round(inp * expand_ratio))

        self.kernel_size = kernel_size
        self.use_res_connect = use_res_connect

        layers = []
        if expand_ratio != 1:
            # pw
            layers.append(ConvBNReLU(inp, hidden_dim, kernel_size=1))
            layers.extend([
            # dw
            ConvBNReLU(hidden_dim, hidden_dim, stride=stride, groups=hidden_dim, kernel_size=self.kernel_size),
            # pw-linear
            nn.Conv2d(hidden_dim, oup, 1, 1, 0, bias=False),
            nn.BatchNorm2d(oup),
        ])
        self.conv = nn.Sequential(*layers)

    def forward(self, x):
        #print (x.shape)
        if self.use_res_connect:
            return x + self.conv(x)
        else:
            return self.conv(x)

class InvertedResidual_3_3(InvertedResidual):
    def __init__(self, in_channels):
        super().__init__(in_channels, in_channels, 3, 1, 3, 1)

class InvertedResidual_3_6(InvertedResidual):
    def __init__(self, in_channels):
        super().__init__(in_channels, in_channels, 3, 1, 6, 1)

class InvertedResidual_5_3(InvertedResidual):
    def __init__(self, in_channels):
        super().__init__(in_channels, in_channels, 5, 1, 3, 1)

class InvertedResidual_5_6(InvertedResidual):
    def __init__(self, in_channels):
        super().__init__(in_channels, in_channels, 5, 1, 6, 1)

class InvertedResidual_3_3_down(InvertedResidual):
    def __init__(self, in_channels):
        super().__init__(in_channels, 2*in_channels, 3, 2, 3, 0)

class InvertedResidual_3_6_down(InvertedResidual):
    def __init__(self, in_channels):
        super().__init__(in_channels, 2*in_channels, 3, 2, 6, 0)

class InvertedResidual_5_3_down(InvertedResidual):
    def __init__(self, in_channels):
        super().__init__(in_channels, 2*in_channels, 5, 2, 3, 0)

class InvertedResidual_5_6_down(InvertedResidual):
    def __init__(self, in_channels):
        super().__init__(in_channels, 2*in_channels, 5, 2, 6, 0)


class Conv_MaxPool(nn.Module):
    def __init__(self, channels):
        super(Conv_MaxPool, self).__init__()

        self.conv = ConvBNReLU(channels, 2*channels, 1, 1, 1)
        self.downsample = nn.MaxPool2d(2, 2)

    def forward(self, x):
        x = self.conv(x)
        x = self.downsample(x)
        return x
