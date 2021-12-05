# imports
import os
import tensorflow.nn as nn
import tensorflow.keras.backend as K
from tensorflow.keras import Input,Model
from tensorflow.keras.utils import plot_model
from tensorflow.keras import layers, regularizers,activations
from utils import DotDict
from models.DGV0.model_v0 import DGM
# end imports



config = DotDict({  'n_filters'     : 128,
                    'kernel'        : 3,
                    'n_res_blocks'  : 6,
                    'n_cbam_blocks' : 8,
                    'l2_reg'        : 0.0001,
                    'dropout'       : 0.2,
                    'repetitions'   : (3,7,3),
                    'groups'        : 8
                })


'''
    -------------------------------------------------------------------------------------------     
        DGM Version 04 CBAM: (DeepGoModel) : in this class we handle all stuff related 
                                        to the deep neural model
                            who will represent our GO player all versions with different 
                            architechtures will inheritat this basic methods and added to them
                            their new specific blocks or methods.
    -------------------------------------------------------------------------------------------
'''
class DGMV5(DGM):
    
    def __init__(self,version=4,n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout) -> None:
        super().__init__(version=version,n_filters=n_filters,kernel_size=kernel_size,
                        l2_reg=l2_reg,dropout=dropout,n_res_blocks=config.n_res_blocks)
                
        self.n_cbam_blocks = config.n_cbam_blocks
        self.repetitions = config.repetitions
        self.groups = config.groups
        self.channels = n_filters

    def body_block(self,x,n_blocks=config.n_cbam_blocks):
        x = self.shufflenet_block(x, channels=self.channels, strides=2, groups=self.groups)
        for _ in range(n_blocks):
            x = self.shufflenet_block(x, channels=self.channels, strides=1, groups=self.groups)
        return x

    def channel_shuffle(self,x, groups):  
        _, width, height, channels = x.get_shape().as_list()
        group_ch = channels // groups

        x = layers.Reshape([width, height, group_ch, groups])(x)
        x = layers.Permute([1, 2, 4, 3])(x)
        x = layers.Reshape([width, height, channels])(x)
        return x
    
    def gconv(self,tensor, channels, groups):
        input_ch = tensor.get_shape().as_list()[-1]
        group_ch = input_ch // groups
        output_ch = channels // groups
        groups_list = []

        for i in range(groups):
            group_tensor = tensor[:, :, :, i * group_ch: (i+1) * group_ch]
            # group_tensor = Lambda(lambda x: x[:, :, :, i * group_ch: (i+1) * group_ch])(tensor)
            group_tensor = layers.Conv2D(output_ch, 1)(group_tensor)
            groups_list.append(group_tensor)

        output = layers.Concatenate()(groups_list)
        return output

    def shufflenet_block(self,tensor, channels, strides, groups):
        x = self.gconv(tensor, channels=channels // 4, groups=groups)
        x = layers.BatchNormalization()(x)
        x = self.activation(x)

        x = self.channel_shuffle(x, groups)
        x = layers.DepthwiseConv2D(kernel_size=3, strides=strides, padding='same')(x)
        x = layers.BatchNormalization()(x)

        if strides == 2:
            channels = channels - tensor.get_shape().as_list()[-1]
        x = self.gconv(x, channels=channels, groups=groups)
        x = layers.BatchNormalization()(x)

        if strides == 1:
            x = layers.add([tensor, x])
        else:
            avg = layers.AvgPool2D(pool_size=3, strides=2, padding='same')(tensor)
            x = layers.Concatenate()([avg, x])

        output = self.activation(x)
        return output

    def activation(self, x):
        return nn.swish(x)
