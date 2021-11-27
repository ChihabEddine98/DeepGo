##############
# Params
#############

class dotdict(dict):
    """dot.notation access to dictionary attributes"""
    __getattr__ = dict.get
    __setattr__ = dict.__setitem__
    __delattr__ = dict.__delitem__


config = dotdict({  'n_planes' : 31,
                    'n_moves'  : 361,
                    'n_samples': 20_000,
                    'dim'      : 19 ,
                    'n_filters'     : 512,
                    'kernel'        : 5,
                    'n_res_blocks'  : 6,
                    'l2_reg'        : 0.0001,
                    'dropout_ratio' : 0.5,
                    'n_epochs'      : 30,
                    'batch_size'    : 64 ,
                    'policy_w'      : 0.5,
                    'value_w'       : 0.5,
                    'lr'            : 0.1,
                    'beta_1'        : 0.9,
                    'beta_2'        : 0.999,
                    'n_btnk_blocks' : 6,
                    'squeeze'       : 128,
                })

# DATA
PLANES = 31
MOVES = 361
N = 1000
DIM = 19

# MODEL
FILTERS = 64
KERNEL_SIZE = (3,3)
RES_BLOCKS = 6

# TRAIN
NB_EPOCHS = 20
BATCH_SIZE = 128

## MODEL V0 : 
'''
   [loss: 2.6539 - policy_loss: 2.3183 - value_loss: 0.6653 - policy_categorical_accuracy: 0.4124 - value_mse: 0.1056 ]
   
   configs : 
            activation : swish
            dropout : 0.3
            l2_reg : 1e-4



'''