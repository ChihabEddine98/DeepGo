# imports
import os
import tensorflow.nn as nn
from tensorflow.keras import Input,Model
from tensorflow.keras.utils import plot_model
from tensorflow.keras import layers, regularizers,activations
from utils import DotDict
from models.DGV0.model_v0 import DGM
# end imports



config = DotDict({  'n_filters'     : 256,
                    'kernel'        : 5,
                    'n_res_blocks'  : 8,
                    'l2_reg'        : 0.0001,
                    'dropout'       : 0.5,
                    'n_btnk_blocks' : 8,
                    'squeeze'       : 128,
                })


'''
    -------------------------------------------------------------------------------------------     
        DGM (DeepGoModel) : in this class we handle all stuff related to the deep neural model
                            who will represent our GO player all versions with different 
                            architechtures will inheritat this basic methods and added to them
                            their new specific blocks or methods.
    -------------------------------------------------------------------------------------------
'''
class DGM_MOBILENET(DGM):
    
    def __init__(self,version=1,n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout,n_btnk_blocks=config.n_btnk_blocks,squeeze=config.squeeze) -> None:
        super().__init__(version=version,n_filters=n_filters,kernel_size=kernel_size,l2_reg=l2_reg,dropout=dropout)

        self.n_btnk_blocks = n_btnk_blocks
        self.squeeze = squeeze

    def body_block(self, x, n_blocks=config.n_btnk_blocks):
        # Bottelneck Blocks
        for _ in range(n_blocks):
             x = self.bottleneck_block(x)
        return x 

    def bottleneck_block(self,x):
        m = layers.Conv2D(self.n_filters,1, kernel_regularizer=self.l2_reg,use_bias=0)(x)
        m = layers.BatchNormalization()(m)
        m = nn.relu(m)
      
        m = layers.DepthwiseConv2D((5,5), padding='same',kernel_regularizer=self.l2_reg,use_bias=0)(m)
        m = layers.BatchNormalization()(m)
        m = nn.relu(m)

        #m = layers.Dropout(self.dropout)(m)
        m = self.sub_residual_block(m)

        m = layers.Conv2D(self.squeeze, 1,kernel_regularizer=self.l2_reg,use_bias=0)(m)
        m = layers.BatchNormalization()(m)

        
        #m = layers.DepthwiseConv2D((5,5), padding='same',kernel_regularizer=self.l2_reg,use_bias=0)(m)
        x = layers.Add()([m, x])
        return x
    
    def sub_residual_block(self,x1,ratio=2):
        x = layers.Dropout(self.dropout)(x1)
        x = layers.GlobalAveragePooling2D()(x)
        x = layers.Dense(self.n_filters//ratio, activation='relu')(x)
        x = layers.Dense(self.n_filters, activation='sigmoid')(x)
        x = layers.Dropout(self.dropout)(x)
        x = layers.Multiply()([x1, x])
        return x