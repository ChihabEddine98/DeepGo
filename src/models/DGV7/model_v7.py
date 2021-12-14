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


config = DotDict({  'n_filters'     : 192,
                    'kernel'        : 5,
                    'n_res_blocks'  : 8,
                    'l2_reg'        : 0.0001,
                    'dropout'       : 0.2,
                    'n_inc_blocks'  : 14,
                    'squeeze'       : 16,
                })

'''
    -------------------------------------------------------------------------------------------     
        DGM (DeepGoModel) : Inception Net with Squeeze & Excitation Blocks / Swish 
    -------------------------------------------------------------------------------------------
'''
class DGMV8(DGM):
    
    def __init__(self,version=8,n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout,n_inc_blocks=config.n_inc_blocks,squeeze=config.squeeze) -> None:
        super().__init__(version=version,n_filters=n_filters,kernel_size=kernel_size,l2_reg=l2_reg,dropout=dropout)

        self.n_inc_blocks = n_inc_blocks
        self.squeeze = squeeze

    def body_block(self, x, n_blocks=config.n_inc_blocks):
        # Inception Blocks
        for _ in range(n_blocks):
            x = self.inception_block(x,[64,32,32,16,32,64],[1,3,5])
            x = self.sub_residual_block(x,ratio=8)
            x = self.activation(x)
            x = layers.BatchNormalization()(x)
        return x
    
    def build_model(self,n_blocks=config.n_inc_blocks):
        return super().build_model(n_blocks)
    


    def inception_block(self,x,filters,kernels,pad='same'):
        t1 = layers.Conv2D(filters[0],kernels[0],padding=pad)(x)
        t1 = self.activation(t1)

        t2 = layers.Conv2D(filters[1],kernels[0],padding=pad)(x) 
        t2 = self.activation(t2)
        t2 = layers.Conv2D(filters[2],kernels[1],padding=pad)(t2)
        t2 = self.activation(t2)

        t3 = layers.Conv2D(filters[3],kernels[0],padding=pad)(x) 
        t3 = self.activation(t3)
        t3 = layers.Conv2D(filters[4],kernels[2],padding='same')(t3)
        t3 = self.activation(t3)
        
        t4 = layers.MaxPool2D(kernels[0],padding='same')(x) 
        t4 = layers.Conv2D(filters[5],kernels[0],padding=pad)(t4)
        t4 = self.activation(t4)
        
        return layers.Concatenate()([t1,t2,t3,t4])


    def output_policy_block(self,x):
        policy_head = layers.Conv2D(1, 1, padding='same', use_bias=False, kernel_regularizer=self.l2_reg)(x)
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