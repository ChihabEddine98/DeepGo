
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
                    'n_samples': 25_000,
                    'n_steps'  : 98,
                    'dim'      : 19 ,
                    'start_epoch'   : 1,
                    'end_epoch'     : 3_000,
                    'n_epochs'      : 3_000,
                    'batch_size'    : 1024 ,
                    'policy_w'      : 1.0,
                    'value_w'       : 1.0,
                    'lr'            : 0.005,
                    'lr_min'        : 5e-7,
                    'beta_1'        : 0.9,
                    'beta_2'        : 0.999,
                    'chkp_frq'      : 1,
                    'print_frq'     : 10,
                    'verbose'       : 1,
                    'save_format'   : 'h5',
                    'devices_'       : ['/device:GPU:0','/device:GPU:1','/device:GPU:2','/device:GPU:3',
                                       '/device:GPU:4','/device:GPU:5','/device:GPU:6','/device:GPU:7'],
                    'devices'       : ['/device:GPU:0','/device:GPU:1'],
                    'n_cycles'      : 1,
                    'annealing'     : 1,
                    'info_style'    : "bold yellow",
                    'succes_style'  : "bold green",
                    'error_style'   : "bold red"   
                })
