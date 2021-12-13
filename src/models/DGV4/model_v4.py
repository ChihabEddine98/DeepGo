# imports
import os
import tensorflow.nn as nn
import tensorflow.keras.backend as K
from tensorflow.keras import Input,Model
from tensorflow.keras.utils import plot_model
from tensorflow.keras import layers, regularizers,activations
from tensorflow.keras.optimizers import Adam
from utils import DotDict,configs
from models.DGV0.model_v0 import DGM
# end imports



config = DotDict({  'n_filters'     : 60,
                    'kernel'        : 5,
                    'n_res_blocks'  : 6,
                    'n_cbam_blocks' : 8,
                    'l2_reg'        : 0.0001,
                    'dropout'       : 0.2,
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
class DGMV4(DGM):
    
    def __init__(self,version=4,n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout) -> None:
        super().__init__(version=version,n_filters=n_filters,kernel_size=kernel_size,
                        l2_reg=l2_reg,dropout=dropout,n_res_blocks=config.n_cbam_blocks)
                
        self.n_cbam_blocks = config.n_cbam_blocks

    def body_block(self,x,n_blocks=config.n_cbam_blocks):
        # CBAM Blocks 
        for _ in range(n_blocks):
            x = self.cbam_block(x)
        return x

    def build_model(self,n_blocks=config.n_cbam_blocks):
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

    def cbam_block(self,x):
        x1 = layers.Conv2D(self.n_filters,3, padding='same')(x)
        x1 = layers.BatchNormalization()(x1)
        x1 = self.activation(x1)

        x1 = layers.Conv2D(self.n_filters, 5, padding='same')(x1)
        x1 = layers.BatchNormalization()(x1)

        x1 = self.channel_attention_module(x1, self.n_filters, ratio=16)
        x1 = self.spatial_attention_module(x1)


        x = layers.add([x1, x])
        x = self.activation(x)
        x = layers.BatchNormalization()(x)

        return x

    def channel_attention_module(self,in_block, filters, ratio):
        maxp = layers.GlobalMaxPooling2D()(in_block)
        avgp = layers.GlobalAveragePooling2D()(in_block)

        hidden_ff = layers.Dense(filters // ratio, activation='swish')
        out_ff = layers.Dense(filters)

        maxp = hidden_ff(maxp)
        maxp = out_ff(maxp)

        avgp = hidden_ff(avgp)
        avgp = out_ff(avgp)

        add_x = layers.add([maxp, avgp])
        activ_x = layers.Activation('sigmoid')(add_x)
        return layers.multiply([in_block, activ_x])

    def spatial_attention_module(self,in_block):
        maxp = layers.Lambda(lambda x: K.mean(x, axis=3, keepdims=True))(in_block)
        avgp = layers.Lambda(lambda x: K.max(x, axis=3, keepdims=True))(in_block)

        max_avg = layers.Concatenate()([maxp, avgp])

        conv_x = layers.Conv2D(1, (7, 7), padding='same')(max_avg)

        activ_x = layers.Activation('sigmoid')(conv_x)
        return layers.multiply([in_block, activ_x])
    
    
    def activation(self, x):
        return x*nn.relu6(x+3)*0.1666666666667
