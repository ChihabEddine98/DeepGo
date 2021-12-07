# imports
import os
import collections
import tensorflow.nn as nn
from tensorflow.keras import Input,Model
from tensorflow.keras.utils import plot_model
from tensorflow.keras import layers, regularizers,activations
from tensorflow.keras.optimizers import SGD,Adam

from models.DGV0.model_v0 import DGM
from utils import configs , DotDict 
# end imports


config = DotDict({  'n_filters'     : 64,
                    'kernel'        : 5,
                    'n_res_blocks'  : 8,
                    'l2_reg'        : 0.0001,
                    'dropout'       : 0.5,
                    'n_btnk_blocks' : 10,
                    'squeeze'       : 16,
                })

BlockArgs = collections.namedtuple('BlockArgs', [
    'kernel_size', 'num_repeat', 'input_filters', 'output_filters',
    'expand_ratio', 'id_skip', 'strides', 'se_ratio'
])

BlockArgs.__new__.__defaults__ = (None,) * len(BlockArgs._fields)

DEFAULT_BLOCKS_ARGS = [
    BlockArgs(kernel_size=3, num_repeat=1, input_filters=32, output_filters=16,
              expand_ratio=1, id_skip=True, strides=[1, 1], se_ratio=0.25),
    BlockArgs(kernel_size=3, num_repeat=2, input_filters=16, output_filters=24,
              expand_ratio=6, id_skip=True, strides=[2, 2], se_ratio=0.25),
    BlockArgs(kernel_size=5, num_repeat=2, input_filters=24, output_filters=40,
              expand_ratio=6, id_skip=True, strides=[2, 2], se_ratio=0.25),
    BlockArgs(kernel_size=3, num_repeat=3, input_filters=40, output_filters=80,
              expand_ratio=6, id_skip=True, strides=[2, 2], se_ratio=0.25),
    BlockArgs(kernel_size=5, num_repeat=3, input_filters=80, output_filters=112,
              expand_ratio=6, id_skip=True, strides=[1, 1], se_ratio=0.25),
    BlockArgs(kernel_size=5, num_repeat=4, input_filters=112, output_filters=192,
              expand_ratio=6, id_skip=True, strides=[2, 2], se_ratio=0.25),
    BlockArgs(kernel_size=3, num_repeat=1, input_filters=192, output_filters=320,
              expand_ratio=6, id_skip=True, strides=[1, 1], se_ratio=0.25)
]

'''
    -------------------------------------------------------------------------------------------     
        DGM (DeepGoModel) : Efficient Net with Squeeze & Excitation Blocks / Swish 
    -------------------------------------------------------------------------------------------
'''
class DGMV2(DGM):
    
    def __init__(self,version=2,n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout,n_btnk_blocks=config.n_btnk_blocks,squeeze=config.squeeze) -> None:
        super().__init__(version=version,n_filters=n_filters,kernel_size=kernel_size,l2_reg=l2_reg,dropout=dropout)

        self.n_btnk_blocks = n_btnk_blocks
        self.squeeze = squeeze

    def body_block(self, x, n_blocks=config.n_btnk_blocks):
        # Bottelneck Blocks
        for _ in range(self.n_btnk_blocks):
            x = self.inception_block(x,[64,48,32,32,48,64],[1,3,5])
            #x = self.se_block(x)
        return x
  
    def inception_block(self,x,filters,kernels):
        t1 = layers.Conv2D(filters[0],kernels[0])(x)
        t1 = self.activation(t1)

        t2 = layers.Conv2D(filters[1],kernels[0])(x) 
        t2 = self.activation(t2)
        t2 = layers.Conv2D(filters[2],kernels[1],padding='same')(t2)
        t2 = self.activation(t2)

        t3 = layers.Conv2D(filters[3],kernels[0])(x) 
        t3 = self.activation(t3)
        t3 = layers.Conv2D(filters[4],kernels[2],padding='same')(t3)
        t3 = self.activation(t3)
        
        t4 = layers.MaxPool2D(kernels[0],padding='same')(x) 
        t4 = layers.Conv2D(filters[5],kernels[0])(t4)
        t4 = self.activation(t4)
        
        return layers.Concatenate()([t1,t2,t3,t4])

    def inverted_residual_block(self,x, expand=64, squeeze=16):
        block = layers.Conv2D(expand, (1,1), activation='relu')(x)
        block = layers.DepthwiseConv2D((3,3),padding='same',activation='relu')(block)
        block = layers.Conv2D(squeeze, (1,1), activation='relu')(block)
        return layers.Add()([block, x])

    def se_block(self,x, filters=192, squeeze_ratio=0.25):
        x_ = layers.GlobalAveragePooling2D()(x)
        x_ = layers.Reshape((1,1,filters))(x_)
        squeezed_filters = max(1, int(filters * squeeze_ratio))
        x_ = layers.Conv2D(squeezed_filters ,1, activation='relu')(x_)
        x_ = layers.Conv2D(filters,1, activation='sigmoid')(x_)
        return layers.Multiply()([x, x_])
    
    def mbConv_block(self,input_data, block_arg):
        kernel_size = block_arg.kernel_size
        num_repeat  = block_arg.num_repeat
        input_filters = block_arg.input_filters
        output_filters = block_arg.output_filters
        expand_ratio = block_arg.expand_ratio
        id_skip = block_arg.id_skip
        strides = block_arg.strides
        se_ratio = block_arg.se_ratio
        expanded_filters = input_filters * expand_ratio

        x = layers.Conv2D(expanded_filters, 1, padding='same', use_bias=0)(input_data)
        x = layers.BatchNormalization()(x)
        x = self.activation(x)

        x = layers.DepthwiseConv2D(kernel_size, strides, padding='same', use_bias=0)(x)
        x = layers.BatchNormalization()(x)
        x = self.activation(x)
        
        #se = self.se_block(input_filters,0.25)
        #x = layers.Multiply([x, se])


        x = layers.Conv2D(output_filters, 1, padding='same', use_bias=0)(x)
        x = layers.BatchNormalization()(x)
        x = self.activation(x)
        return x

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
    
    def build_model(self,n_blocks=config.n_btnk_blocks):
        # Input Block
        inp = Input(shape=(self.dim, self.dim, self.n_planes), name='board')
        x = self.input_block(inp)
        

        # Body Block 
        x = self.body_block(x,n_blocks)

        # Outputs blocks
        policy_head = self.output_policy_block(x)
        value_head = self.output_value_block(x)

        # Build model 
        self.model = Model(inputs=inp, outputs=[policy_head, value_head])

        self.model.compile(
                optimizer = Adam(lr=configs.lr),
                loss={'policy': 'categorical_crossentropy', 'value': 'binary_crossentropy'},
                loss_weights={'policy' : configs.policy_w, 'value' : configs.value_w},
                metrics={'policy': 'categorical_accuracy', 'value': 'mse'})

        return self.model
