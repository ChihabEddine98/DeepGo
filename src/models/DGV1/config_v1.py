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
                    'n_samples': 60_000,
                    'dim'      : 19 ,
                    'n_filters'     : 256,
                    'kernel'        : 5,
                    'n_res_blocks'  : 8,
                    'l2_reg'        : 0.0001,
                    'dropout_ratio' : 0.5,
                    'n_epochs'      : 20,
                    'batch_size'    : 128 ,
                    'policy_w'      : 1.0,
                    'value_w'       : 1.0,
                    'lr'            : 0.01,
                    'beta_1'        : 0.9,
                    'beta_2'        : 0.999,
                    'n_btnk_blocks' : 8,
                    'squeeze'       : 128,
                    'chkp_frq'      : 2,
                    'print_frq'     : 5,
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