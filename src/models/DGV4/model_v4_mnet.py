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
                    'n_res_blocks'  : 6,
                    'n_btk_blocks'  : 16,
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
                ,dropout=config.dropout,n_btk_blocks=config.n_btk_blocks) -> None:
        super().__init__(version=version,n_filters=n_filters,kernel_size=kernel_size,
                        l2_reg=l2_reg,dropout=dropout,n_res_blocks=config.n_btk_blocks)
        
        self.n_btk_blocks = n_btk_blocks
        #self.n_cbam_blocks = config.n_cbam_blocks
        self.repetitions = config.repetitions
        self.groups = config.groups
        self.channels = n_filters
        self.squeeze = config.squeeze

    def build_model(self,n_blocks=config.n_btk_blocks):
        return super().build_model(n_blocks)
    
    def input_block(self,inp,kernel_resize=5,pad='same'):
        # CONV2D + BN + activation 
        x = layers.Conv2D(self.squeeze, 1, padding=pad)(inp)
        x = layers.BatchNormalization()(x)
        x = self.activation(x)
        
        # CONV2D (resize) + BN + activation
        x1 = layers.Conv2D(self.squeeze,kernel_resize, padding=pad)(inp)
        x1 = layers.BatchNormalization()(x1)
        x1 = self.activation(x1)

        x = layers.add([x, x1])
        
        return x
    
    def body_block(self,x,n_blocks=config.n_btk_blocks):
        for _ in range(n_blocks):
            x = self.bottleneck_block(x)
        return x

    def bottleneck_block(self,x):
        m = layers.Conv2D(self.n_filters,1, kernel_regularizer=self.l2_reg,use_bias=0)(x)
        m = layers.BatchNormalization()(m)
        m = self.activation(m)
      
        m = layers.DepthwiseConv2D(self.kernel, padding='same',kernel_regularizer=self.l2_reg,use_bias=0)(m)
        m = layers.BatchNormalization()(m)
        m = self.activation(m)

        #m = layers.Dropout(self.dropout)(m)
        #m = self.sub_residual_block(m)
        m = self.channel_attention_module(m, self.n_filters, ratio=8)

        m = layers.Conv2D(self.squeeze, 1,kernel_regularizer=self.l2_reg,use_bias=0)(m)
        m = layers.BatchNormalization()(m)
        m = self.activation(m)



        #m = layers.DepthwiseConv2D((5,5), padding='same',kernel_regularizer=self.l2_reg,use_bias=0)(m)
        x = layers.add([m, x])

        return x

    def channel_attention_module(self,in_block, filters, ratio):
        maxp = layers.GlobalMaxPooling2D()(in_block)
        avgp = layers.GlobalAveragePooling2D()(in_block)

        hidden_ff = layers.Dense(filters // ratio, activation='relu')
        out_ff = layers.Dense(filters)

        maxp = hidden_ff(maxp)
        maxp = out_ff(maxp)

        avgp = hidden_ff(avgp)
        avgp = out_ff(avgp)

        add_x = layers.add([maxp, avgp])
        activ_x = layers.Activation('sigmoid')(add_x)
        return layers.multiply([in_block, activ_x])

    def activation(self, x):
        return nn.swish(x)
