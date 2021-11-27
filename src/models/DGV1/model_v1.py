# imports
import os
import tensorflow.nn as nn
from tensorflow.keras import Input,Model
from tensorflow.keras.utils import plot_model
from tensorflow.keras import layers, regularizers,activations

from config import config 
from models.DGV0.model import DGM
# end imports




'''
    -------------------------------------------------------------------------------------------     
        DGM (DeepGoModel) : in this class we handle all stuff related to the deep neural model
                            who will represent our GO player all versions with different 
                            architechtures will inheritat this basic methods and added to them
                            their new specific blocks or methods.
    -------------------------------------------------------------------------------------------
'''
class DGM_MOBILENET(DGM):
    
    def __init__(self,version=1,dim=config.dim,n_moves=config.n_moves,n_planes=config.n_planes,
                n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout_ratio,n_btnk_blocks=config.n_btnk_blocks,squeeze=config.squeeze) -> None:
        super().__init__(version=version,
                         dim=dim,
                         n_moves=n_moves,
                         n_planes=n_planes,
                         n_filters=n_filters,
                         kernel_size=kernel_size,
                         l2_reg=l2_reg,
                         dropout=dropout)

        self.n_btnk_blocks = n_btnk_blocks
        self.squeeze = squeeze
        self.model = None

    def build_model(self):
        # Input Block
        inp = Input(shape=(self.dim, self.dim, self.n_planes), name='board')
        x = self.input_block(inp)


        # Residual Blocks 
        for _ in range(self.n_btnk_blocks):
            x = self.bottleneck_block(x)
        
        

        # Outputs blocks
        policy_head = self.output_policy_block(x)
        value_head = self.output_value_block(x)

        # Build model 
        self.model = Model(inputs=inp, outputs=[policy_head, value_head])
        return self.model

    def input_block(self,inp,kernel_resize=5,pad='same'):
      # CONV2D + BN + activation 
      x = layers.Conv2D(self.squeeze, 1, padding=pad)(inp)
      x = layers.BatchNormalization()(x)
      x = self.activation(x)
      x = layers.DepthwiseConv2D((5,5), padding='same',kernel_regularizer=self.l2_reg,use_bias=0)(x)

      if not kernel_resize:
          return x
      
      # CONV2D (resize) + BN + activation
      x1 = layers.Conv2D(self.squeeze,kernel_resize, padding=pad)(inp)
      x1 = layers.BatchNormalization()(x1)
      x = layers.add([x, x1])
      return x
      
    def bottleneck_block(self,x):
        m = layers.Conv2D(self.n_filters,1, kernel_regularizer=self.l2_reg,use_bias=0)(x)
        m = layers.BatchNormalization()(m)
        m = self.activation(m)
        m = layers.DepthwiseConv2D((3,3), padding='same',kernel_regularizer=self.l2_reg,use_bias=0)(m)
        m = layers.BatchNormalization()(m)
        m = self.activation(m)
      

        m = layers.Conv2D(self.squeeze, 1,kernel_regularizer=self.l2_reg,use_bias=0)(m)

        m = self.sub_residual_block(m)
        
        m = layers.DepthwiseConv2D((5,5), padding='same',kernel_regularizer=self.l2_reg,use_bias=0)(m)
        x = layers.Add()([m, x])
        return x
    
    def sub_residual_block(self,x1,ratio=2):
        x = layers.Dropout(self.dropout)(x1)
        x = layers.GlobalAveragePooling2D()(x)
        x = layers.Dense(self.n_filters//ratio, activation='relu')(x)
        x = layers.Dense(self.squeeze, activation='sigmoid')(x)
        x = layers.Dropout(self.dropout)(x)
        x = layers.Multiply()([x1, x])
        return x

    def output_value_block(self,x):
      value_head = layers.GlobalAveragePooling2D()(x)
      value_head = layers.Dense(50, kernel_regularizer=self.l2_reg)(value_head)
      value_head = layers.BatchNormalization()(value_head)
      value_head = self.activation(value_head)
      value_head = layers.Dropout(self.dropout)(value_head)
      value_head = layers.Dense(1, activation='sigmoid', name='value', kernel_regularizer=self.l2_reg)(value_head)
      return value_head

    def activation(self,x):
        return nn.swish(x)