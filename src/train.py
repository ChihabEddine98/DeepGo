import os 
import argparse
import tensorflow as tf
import tensorflow.keras as keras
from rich.console import Console
from rich.markdown import Markdown


#from models.DGV2.model_v2 import DGMV2
from models.DGV2.model_v2_1 import DGMV2_1
from models.DGV4.model_v4 import DGMV4
from models.DGV4.model_v4_mnet import DGMV5
from models.DGV5.model_v5 import DGMV2
from models.DGV6.model_v6 import DGMV6
from models.DGV7.model_v7 import DGMV8
from models.DGV8.model_v8 import DGMV9


from trainer import Trainer
from utils import configs




def console_handler(console):
    '''
         ---------------------------------------------------------------
                    Command Line Guide (args to use)
         ---------------------------------------------------------------    

        -h    [–-help]          : help with command line 
        -gpu  [–-num-gpu]       : # of GPUs to be used for training [default : 2]
        -s    [--start-epoch]   : the epoch to porsuit the training from  [default : 1]
        -e    [--end-epoch]     : the last epoch of training [default : 500]
        -b    [--batch-size]    : the batch size used to train model [default : 512]
        -n    [--num-samples]   : # of samples in the get_batch per epoch [default : 25k]
        -w    [--show-warnings] : the warnings show level [from 0 to 3]
        -l    [--load][PATH]    : load Model to continue training from [PATH]
    '''
    # Get All Arguments from Command Line 
    parser = argparse.ArgumentParser(description='Argument Parser For DGM models.')
    parser.add_argument('-gpu','--num-gpu',type=int,default=2,metavar='',help="the # of GPUs used to train the model. [default : 2]")
    parser.add_argument('-s','--start-epoch',type=int,default=1,metavar='',help="the epoch to porsuit the training from  [default : 1]")
    parser.add_argument('-e','--end-epoch',type=int,default=500,metavar='',help="the last epoch of training [default : 500]")
    parser.add_argument('-b','--batch-size',type=int,default=512,metavar='',help="the batch size used to train model [default : 512]")
    parser.add_argument('-n','--num-samples',type=int,default=25_000,metavar='',help="# of samples in the get_batch per epoch [default : 25k]")
    parser.add_argument('-w','--show-warnings',type=int,default=2,metavar='',help="the warnings show level [from 0 to 3]")
    parser.add_argument('-l','--load',type=str,default='',help="load Model to continue training")

    args = parser.parse_args()
    
    # Set Parsed Parameters into configs
    configs.devices = [f"/device:GPU:{i}" for i in range(args.num_gpu)]
    configs.start_epoch = args.start_epoch
    configs.end_epoch = args.end_epoch
    configs.batch_size = args.batch_size
    configs.n_samples = args.num_samples
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = str(args.show_warnings)


    console.print(f" Here are your `configs` : {configs}")

    # Make it Parallel 🥳
    strategy = tf.distribute.MirroredStrategy(configs.devices)
    return strategy,args




if __name__ == '__main__':

    console = Console()
    strategy , args = console_handler(console)
    title = Markdown(f"# Start training ON {len(configs.devices)} GPU's", style=configs.info_style)
    console.print(title)

    # Build the model 
    with strategy.scope():
        dgm = DGMV9() 
        # Check if we wanna load an existing model and continue train
        if args.load != '':
            model = keras.models.load_model(args.load)
        else:
            model = dgm.build_model() 

        dgm.model = model
        dgm.summary()

    # Get Trainer
    trainer = Trainer(dgm)

    # Train
    history = trainer.train()

