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



config = DotDict({  'n_filters'     : 256,
                    'squeeze'       : 64,
                    'kernel'        : 3,
                    'pool'          : 3 ,
                    'n_res_blocks'  : 6,
                    'n_btk_blocks'  : 8,
                    'l2_reg'        : 0.0001,
                    'dropout'       : 0.2,
                    'repetitions'   : (3,7,3),
                    'groups'        : 8
                })


'''
    -------------------------------------------------------------------------------------------     
        DGM Version 06 MxNet: (DeepGoModel) : in this class we handle all stuff related 
                                        to the deep neural model
                            who will represent our GO player all versions with different 
                            architechtures will inheritat this basic methods and added to them
                            their new specific blocks or methods.
    -------------------------------------------------------------------------------------------
'''
class DGMV6(DGM):
    
    def __init__(self,version=7,n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout,n_btk_blocks=config.n_btk_blocks) -> None:
        super().__init__(version=version,n_filters=n_filters,kernel_size=kernel_size,
                        l2_reg=l2_reg,dropout=dropout,n_res_blocks=config.n_btk_blocks)
        
        self.n_btk_blocks = n_btk_blocks
        #self.n_cbam_blocks = config.n_cbam_blocks
        self.repetitions = config.repetitions
        self.groups = config.groups
        self.channels = n_filters
        self.pool = config.pool
        self.squeeze = config.squeeze

    def build_model(self,n_blocks=config.n_btk_blocks):
        return super().build_model(n_blocks)
    


    def input_block(self,x):
        x = self.conv_bn(x, filters=self.n_filters//8, kernel_size=self.kernel, strides=2)
        x = self.activation(x)
        x = self.conv_bn(x, filters=self.n_filters//4, kernel_size=self.kernel)
        tensor = self.activation(x)

    
        x = self.sep_bn(tensor, filters=self.n_filters//2, kernel_size=self.kernel)
        x = self.activation(x)

        x = self.sep_bn(x, filters=self.n_filters//2, kernel_size=self.kernel)
        x = layers.MaxPool2D(pool_size=self.pool, strides=2, padding='same')(x)
    
        tensor = self.conv_bn(tensor, filters=self.n_filters//2, kernel_size=1, strides=2)
    
        x = layers.add([tensor, x])
        x = self.activation(x)
        x = self.sep_bn(x, filters=self.n_filters, kernel_size=self.kernel)
        x = self.activation(x)

        x = self.sep_bn(x,filters=self.n_filters, kernel_size=self.kernel)
        x = layers.MaxPool2D(pool_size=self.pool, strides=2, padding='same')(x)
    
        tensor = self.conv_bn(tensor, filters=self.n_filters, kernel_size=1, strides=2)
    
        x = layers.add([tensor, x])
    
        return x

    def residual_block(self, tensor, pad='same'):

        x = self.activation(tensor)
        x = self.sep_bn(x, filters=self.n_filters, kernel_size=self.kernel)
        x = self.activation(x)
        x = self.sep_bn(x, filters=self.n_filters, kernel_size=self.kernel)
        x = self.activation(x)
        x = self.sep_bn(x, filters=self.n_filters, kernel_size=self.kernel)
 
        tensor = layers.add([tensor, x])
 
        return tensor


    def conv_bn(self,x, filters, kernel_size, strides=1):
        x = layers.Conv2D(filters=filters,
                          kernel_size=kernel_size,
                          strides=strides,
                          padding='same',
                          use_bias=False)(x)
        return layers.BatchNormalization()(x)
    
    def sep_bn(self,x, filters, kernel_size, strides=1):
        x = layers.SeparableConv2D(filters=filters,
                            kernel_size=kernel_size,
                            strides=strides,
                            padding='same',
                            use_bias=False)(x)
        return layers.BatchNormalization()(x)

    def activation(self, x):
        return nn.swish(x) 
