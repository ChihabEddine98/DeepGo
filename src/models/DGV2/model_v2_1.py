# imports
import os
import tensorflow.nn as nn
from tensorflow.keras import Input,Model
from tensorflow.keras.utils import plot_model
from tensorflow.keras import layers, regularizers,activations
from utils import DotDict
from models.DGV0.model_v0 import DGM
# end imports



config = DotDict({  'n_filters'     : 89,
                    'kernel'        : 3,
                    'n_res_blocks'  : 6,
                    'l2_reg'        : 0.0001,
                    'dropout'       : 0.2,
                })


'''
    -------------------------------------------------------------------------------------------     
        DGM Version 01: (DeepGoModel) : in this class we handle all stuff related 
                                        to the deep neural model
                            who will represent our GO player all versions with different 
                            architechtures will inheritat this basic methods and added to them
                            their new specific blocks or methods.
    -------------------------------------------------------------------------------------------
'''
class DGMV2_1(DGM):
    
    def __init__(self,version=3,n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout) -> None:
        super().__init__(version=version,n_filters=n_filters,kernel_size=kernel_size,
                        l2_reg=l2_reg,dropout=dropout,n_res_blocks=config.n_res_blocks)

    def input_block(self,inp,kernel_resize=5,pad='same'):
        # CONV2D + BN + activation 
        x = layers.Conv2D(self.n_filters, 3, padding=pad)(inp)
        x = layers.BatchNormalization()(x)
        x = self.activation(x)
        
        # CONV2D (resize) + BN + activation
        x1 = layers.Conv2D(self.n_filters,kernel_resize, padding=pad)(inp)
        x1 = layers.BatchNormalization()(x1)
        x1 = self.activation(x1)

        x = layers.add([x, x1])
        
        return x
    
    def residual_block(self,x):
        x1 = layers.Conv2D(self.n_filters, (3, 3), padding='same')(x)
        x1 = layers.BatchNormalization()(x1)
        x1 = self.activation(x1)

        x1 = layers.Conv2D(self.n_filters, (3, 3), padding='same')(x1)
        x1 = layers.BatchNormalization()(x1)

        x1 = self.se_block(x1, self.n_filters, ratio=4)

        x = layers.add([x1, x])
        x1 = self.activation(x1)
        x = layers.BatchNormalization()(x)

        return x

    def se_block(self,in_block, ch, ratio=16):
        x = layers.Dropout(0.2)(in_block)
        x = layers.GlobalAveragePooling2D()(x)
        x = layers.Dense(ch//ratio, activation='relu')(x)
        x = layers.Dense(ch, activation='sigmoid')(x)
        return layers.Multiply()([in_block, x])
    
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
    
    def activation(self, x):
        return x * nn.relu6(x+3) * 0.166666666667
