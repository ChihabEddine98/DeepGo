
class DotDict(dict):
    '''
        A dict that supports dot notation (recursrively)

        usage: d = DotDict() or d = DotDict({'k1':'v1'})
        
        -- set attributes: d.k2 = v2 or d[k2] = v2
        -- get attributes: d.k2 or d[k2]
    '''
    __getattr__ = dict.__getitem__
    __setattr__ = dict.__setitem__
    __delattr__ = dict.__delitem__

    def __init__(self, dct):
        for key, value in dct.items():
            if hasattr(value, 'keys'):
                value = DotDict(value)
            self[key] = value



configs = DotDict({ 'n_planes' : 31,
                    'n_moves'  : 361,
                    'n_samples': 10_000,
                    'dim'      : 19 ,
                    'n_epochs'      : 30,
                    'batch_size'    : 64 ,
                    'policy_w'      : 0.5,
                    'value_w'       : 0.5,
                    'lr'            : 0.05,
                    'beta_1'        : 0.9,
                    'beta_2'        : 0.999,
                    'chkp_frq'      : 2,
                    'print_frq'     : 5,
                    'verbose'       : 1,
                    'device'        : '/device:GPU:0',
                    'save_format'   : 'h5'
                })
