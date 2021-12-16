# imports
import os
import tensorflow.nn as nn
from tensorflow.keras import Input,Model
from tensorflow.keras.utils import plot_model
from tensorflow.keras import layers, regularizers,activations
from tensorflow.keras.optimizers import SGD,Adam
from utils import configs , DotDict 
# end imports



'''
    Model Configurations :
'''

config = DotDict({  'n_filters'     : 64,
                    'kernel'        : 3,
                    'n_res_blocks'  : 6,
                    'l2_reg'        : 0.0001,
                    'dropout' : 0.2
                })


'''
    -------------------------------------------------------------------------------------------     
        DGM (DeepGoModel) : in this class we handle all stuff related to the deep neural model
                            who will represent our GO player all versions with different 
                            architechtures will inheritat this basic methods and added to them
                            their new specific blocks or methods.
    -------------------------------------------------------------------------------------------
'''
class DGM(object):
    
    def __init__(self,version=0,dim=configs.dim,n_moves=configs.n_moves,n_planes=configs.n_planes,
                n_filters=config.n_filters,kernel_size=config.kernel,l2_reg=config.l2_reg
                ,dropout=config.dropout,n_res_blocks=config.n_res_blocks) -> None:
        super().__init__()
        self.version = version
        self.dim = dim
        self.n_moves = n_moves
        self.n_planes = n_planes
        self.n_filters = n_filters
        self.kernel = kernel_size
        self.l2_reg = regularizers.l2(l2_reg)
        self.dropout = dropout
        self.n_res_blocks = n_res_blocks
        self.model = None
    
    def __str__(self) -> str:
        return f'DGMV{self.version}'

    def summary(self):
        self.model.summary()
    
    def plot_model(self,save_path='models/model_imgs'):
        
        if not self.model:
            print(f' You should build the model first !')
            return
  
        to_file = os.path.join(os.getcwd(),save_path,f'{str(self)}.png')
        plot_model(self.model,to_file=to_file,show_shapes=True)
        
    def build_model(self,n_blocks=config.n_res_blocks):
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
                optimizer = Adam(learning_rate=configs.lr),
                loss={'policy': 'categorical_crossentropy', 'value': 'binary_crossentropy'},
                loss_weights={'policy' : configs.policy_w, 'value' : configs.value_w},
                metrics={'policy': 'categorical_accuracy', 'value': 'mse'})

        return self.model
    
    def input_block(self,inp,kernel_resize=3,pad='same'):
        # CONV2D + BN + activation 
        x = layers.Conv2D(self.n_filters, 1, padding=pad)(inp)
        x = layers.BatchNormalization()(x)
        x = self.activation(x)

        if not kernel_resize:
            return x
        
        # CONV2D (resize) + BN + activation
        x1 = layers.Conv2D(self.n_filters,kernel_resize, padding=pad)(inp)
        x1 = layers.BatchNormalization()(x1)
        x1 = self.activation(x1)
        x = layers.add([x, x1])
        
        return x

    def body_block(self,x,n_blocks=config.n_res_blocks):
        # Residual Blocks 
        for _ in range(n_blocks):
            x = self.residual_block(x)
        return x


    def output_policy_block(self,x):
        policy_head = layers.Conv2D(1, 1, padding='same', use_bias=False, kernel_regularizer=self.l2_reg)(x)
        policy_head = layers.BatchNormalization()(policy_head)
        policy_head = self.activation(policy_head)
        policy_head = layers.Flatten()(policy_head)
        policy_head = layers.Activation('softmax', name='policy')(policy_head)
        return policy_head
        
    def output_value_block(self,x):
        value_head = layers.GlobalAveragePooling2D()(x)
        value_head = layers.Dense(self.n_filters, kernel_regularizer=self.l2_reg)(value_head)
        value_head = layers.BatchNormalization()(value_head)
        value_head = self.activation(value_head)
        value_head = layers.Dropout(self.dropout)(value_head)
        value_head = layers.Dense(1, activation='sigmoid', name='value', kernel_regularizer=self.l2_reg)(value_head)
        return value_head
    
    def sub_residual_block(self,x1,ratio=4):
        x = layers.Dropout(self.dropout)(x1)
        x = layers.GlobalAveragePooling2D()(x)
        x = layers.Dense(self.n_filters//ratio, activation='relu')(x)
        x = layers.Dense(self.n_filters, activation='sigmoid')(x)
        return layers.Multiply()([x1, x])

    def residual_block(self,x,pad='same'):
        x1 = layers.Conv2D(self.n_filters, self.kernel, padding=pad)(x)
        x1 = layers.BatchNormalization()(x1)
        x1 = self.activation(x1)
        x1 = layers.Conv2D(self.n_filters, self.kernel, padding=pad)(x1)
        x1 = layers.BatchNormalization()(x1)

        x1 = self.sub_residual_block(x1)
        
        x = layers.add([x1, x])
        
        x = self.activation(x)
        x = layers.BatchNormalization()(x)
        return x

    def activation(self,x):
        return nn.swish(x)
