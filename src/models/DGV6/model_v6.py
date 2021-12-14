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
                    'kernel'        : 5,
                    'pool'          : 3 ,
                    'n_res_blocks'  : 6,
                    'n_btk_blocks'  : 17,
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
    
    def body_block(self,x,n_blocks=config.n_btk_blocks):
        
        for _ in range(n_blocks):
            x = self.bottleneck_block(x)
        return x

    def bottleneck_block(self,x, expand=config.n_filters, squeeze=config.squeeze):
        m = layers.Conv2D(expand, 1,kernel_regularizer=self.l2_reg,use_bias = 0)(x)
        m = layers.BatchNormalization()(m)
        m = self.activation(m)
        m = layers.DepthwiseConv2D(self.kernel, padding='same',kernel_regularizer=self.l2_reg,use_bias = 0)(m)
        m = layers.BatchNormalization()(m)
        m = self.activation(m)

        m = self.sub_residual_block(m,ratio=16)

        m = layers.Conv2D(squeeze, 1,kernel_regularizer=self.l2_reg,use_bias = 0)(m)
        m = layers.BatchNormalization()(m)

        return layers.add([m, x])

    def input_block(self,inp,kernel_resize=7,pad='same'):
        # CONV2D + BN + activation 
        x = layers.Conv2D(self.squeeze, 5, padding=pad)(inp)
        x = layers.BatchNormalization()(x)
        x = self.activation(x)

        
        # CONV2D (resize) + BN + activation
        x1 = layers.Conv2D(self.squeeze,kernel_resize, padding=pad)(inp)
        x1 = layers.BatchNormalization()(x1)
        x1 = self.activation(x1)
        x = layers.add([x, x1])
        
        return x
    
    def sub_residual_block(self,x1,ratio=16):
        x = layers.Dropout(self.dropout)(x1)
        x = layers.GlobalAveragePooling2D()(x)
        x = layers.Dense(self.n_filters//ratio, activation='relu')(x)
        x = layers.Dense(self.n_filters, activation='sigmoid')(x)
        return layers.Multiply()([x1, x])

    def output_value_block(self,x):
        value_head = layers.GlobalAveragePooling2D()(x)
        value_head = layers.Dense(self.squeeze, kernel_regularizer=self.l2_reg)(value_head)
        value_head = layers.BatchNormalization()(value_head)
        value_head = self.activation(value_head)
        value_head = layers.Dropout(self.dropout)(value_head)
        value_head = layers.Dense(1, activation='sigmoid', name='value', kernel_regularizer=self.l2_reg)(value_head)
        return value_head

    def activation(self, x):
        return nn.swish(x)
