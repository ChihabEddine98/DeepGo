# imports
import os
import collections
import tensorflow.nn as nn
from tensorflow.keras import Input,Model
from tensorflow.keras.utils import plot_model
from tensorflow.keras import layers, regularizers,activations
from utils import DotDict,configs
from models.DGV0.model_v0 import DGM
# end imports

# Train : python train.py -gpu 2 -s 1 -e 1200 -b 1024
config = DotDict({  'n_filters'     : 165,
                    'kernel'        : 5,
                    'n_res_blocks'  : 8,
                    'l2_reg'        : 0.0001,
                    'dropout'       : 0.2,
                    'n_inc_blocks'  : 14,
                    'squeeze'       : 16,
                    'arch': [2,3,4,2,3,1]
                })

'''
    -------------------------------------------------------------------------------------------     
        DGM (DeepGoModel) : Mnasnet / Swish 
    -------------------------------------------------------------------------------------------
'''
class DGMV9(DGM):
    
    def __init__(self,version=9,n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout,n_inc_blocks=config.n_inc_blocks,squeeze=config.squeeze,arch = config.arch) -> None:
        super().__init__(version=version,n_filters=n_filters,kernel_size=kernel_size,l2_reg=l2_reg,dropout=dropout)

        self.n_inc_blocks = n_inc_blocks
        self.squeeze = squeeze
        self.arch = arch


    def body_block(self, x,n_blocks=config.n_inc_blocks):
        # Mixed kernels Blocks
        x = self.sep_conv(x)
        
        # MBConv 3x3
        for _ in range(self.arch[0]):
            x = self.mb_conv(x,k=3,se=0)
        
        # MBConv 5x5 (With SE)
        for _ in range(self.arch[1]):
            x = self.mb_conv(x,k=5,se=1)

        # MBConv 3x3 (No SE)
        for _ in range(self.arch[2]):
            x = self.mb_conv(x,k=3,se=0)
            
        # MBConv 3x3 (SE)
        for _ in range(self.arch[3]):
            x = self.mb_conv(x,k=3,se=1)
            
        # MBConv 5x5 (With SE)
        for _ in range(self.arch[4]):
            x = self.mb_conv(x,k=5,se=1)
            
        # MBConv 3x3 (No SE)
        for _ in range(self.arch[5]):
            x = self.mb_conv(x,k=3,se=0)
        
        return x
        
        
    # MBConv(kernel : 5x5)
    def mb_conv(self,x,se=1,k=3,r=16,pad='same'):
        x_ = layers.Conv2D(self.n_filters,1,padding=pad,kernel_regularizer=self.l2_reg,use_bias=0)(x)
        x_ = self.bn_activation(x_)
        
        x_ = layers.DepthwiseConv2D(k,padding=pad,kernel_regularizer=self.l2_reg,use_bias=0)(x_)
        x_ = self.bn_activation(x_)
        
        if se:
            x_ = self.sub_residual_block(x_,ratio=r)
        
        x_ = layers.Conv2D(self.n_filters, 1,kernel_regularizer=self.l2_reg,use_bias=0)(x_)
        x_ = layers.BatchNormalization()(x_)
        
        x = layers.add([x_, x])
        return x
    
    # MBConv(kernel : 3x3)
    def mb_conv3(self,x,r=4,pad='same'):
        x_ = layers.Conv2D(self.n_filters,1,padding=pad,kernel_regularizer=self.l2_reg,use_bias=0)(x)
        x_ = self.bn_activation(x_)
        
        x_ = layers.DepthwiseConv2D(3,padding=pad,kernel_regularizer=self.l2_reg,use_bias=0)(x_)
        x_ = self.bn_activation(x_)
        
        x_ = layers.Conv2D(self.n_filters, 1,kernel_regularizer=self.l2_reg,use_bias=0)(x_)
        x_ = layers.BatchNormalization()(x_)
        
        x = layers.add([x_, x])
        return x
        
    def sep_conv(self,x,k=3,pad='same'):
        x = layers.DepthwiseConv2D(k,padding=pad,kernel_regularizer=self.l2_reg,use_bias=0)(x)
        x = self.bn_activation(x)
        x = layers.Conv2D(self.n_filters, 1,kernel_regularizer=self.l2_reg,use_bias=0)(x)
        x = layers.BatchNormalization()(x)
        
        return x
        
    def bn_activation(self,x):
        return self.activation(layers.BatchNormalization()(x))

    def output_policy_block(self,x):
        policy_head = layers.Conv2D(1, 1, padding='same', use_bias=0, kernel_regularizer=self.l2_reg)(x)
        policy_head = self.activation(policy_head)
        policy_head = layers.BatchNormalization()(policy_head)
        policy_head = layers.Flatten()(policy_head)
        policy_head = layers.Activation('softmax', name='policy')(policy_head)
        return policy_head

    def output_value_block(self,x):
        value_head = layers.GlobalAveragePooling2D()(x)
        value_head = layers.Dense(50, kernel_regularizer=self.l2_reg)(value_head)
        value_head = self.activation(value_head)
        value_head = layers.BatchNormalization()(value_head)
        value_head = layers.Dropout(self.dropout)(value_head)
        value_head = layers.Dense(1, activation='sigmoid', name='value', kernel_regularizer=self.l2_reg)(value_head)
        return value_head

    def activation(self,x):
        return nn.swish(x)