import pickle
import gc

from math import cos,floor,pi

from keras.callbacks import Callback
from keras import backend as K

from rich.console import Console
from rich.markdown import Markdown
from rich.progress import track

import golois
from utils import configs
from dataloader import DataHandler



# calculate learning rate for an epoch
'''
def cosine_annealing(epoch, n_epochs,n_steps, n_cycles, lr_max):
    epochs_per_cycle = floor(n_epochs/n_cycles)
    cos_inner = (pi * (epoch % epochs_per_cycle)) / (n_steps* epochs_per_cycle)
    return lr_max / 2 * (cos(cos_inner) + 1)
'''
def cosine_annealing(epoch,lr_min,lr_max,n_epochs, n_cycles):
    epochs_per_cycle = floor(n_epochs/n_cycles)
    cos_inner = (pi * (epoch % epochs_per_cycle)) / (epochs_per_cycle)
    lr = lr_min + 0.5 * (lr_max - lr_min) * (1 + cos(cos_inner))
    return lr

class Scheduler(Callback):
	# constructor
	def __init__(self, n_epochs, n_cycles, lrate_max, verbose=0):
		self.epochs = n_epochs
		self.cycles = n_cycles
		self.lr_max = lrate_max
		self.lrates = list()
 
	# calculate learning rate for an epoch
	def cosine_annealing(self, epoch, n_epochs, n_cycles, lrate_max):
		epochs_per_cycle = floor(n_epochs/n_cycles)
		cos_inner = (pi * (epoch % epochs_per_cycle)) / (epochs_per_cycle)
		return lrate_max/2 * (cos(cos_inner) + 1)
 
	# calculate and set learning rate at the start of the epoch
	def on_epoch_begin(self, epoch, logs=None):
		# calculate learning rate
		lr = self.cosine_annealing(epoch, self.epochs, self.cycles, self.lr_max)
		# set learning rate
		K.set_value(self.model.optimizer.lr, lr)
		# log value
		self.lrates.append(lr)

class Trainer(object): 
    
    def __init__(self,model,config=configs) -> None:
        super().__init__()
        self.nn = model
        self.model = model.model 
        self.config = config
        self.model_path = f'{str(model)}.{config.save_format}' 
        self.hist_path = f'{str(model)}_history'
        self.data_loader = DataHandler()
    
    def train(self):
        console = Console()
        # Get Initial Data
        input_data , policy , value , end , groups = self.data_loader.get_data() 

        # Get Validation Data
        title = Markdown(f"# Getting `validation.data`", style=self.config.succes_style)
        console.print(title)

        for _ in track(range(100)):
            golois.getValidation(input_data, policy, value, end)

        histories = {} 
        val_hist = []
        K.set_value(self.model.optimizer.lr, self.config.lr)
        for epoch in range (self.config.start_epoch, self.config.end_epoch + 1):
            title = Markdown(f"## ----- epoch [{epoch}/{self.config.end_epoch}] -----", style=self.config.info_style)
            console.print(title)
            #print (f' Epoch [{epoch}/{self.config.n_epochs}]')
            golois.getBatch(input_data, policy, value, end, groups, epoch*self.config.n_samples)
            #with tf.device(self.config.device):
            if self.config.annealing:
                
                lr = cosine_annealing(epoch=epoch,lr_min=self.config.lr_min,
                                      lr_max=self.config.lr,n_epochs=self.config.n_epochs,
	                                 n_cycles=self.config.n_cycles)


                #lr = K.eval(self.model.optimizer.lr)
                
                '''
                if  epoch < 100:
                     lr = 0.005
                elif epoch > 100 and epoch < 150 :
                     lr = 0.00425
                elif epoch >= 150 and epoch < 200 :
                     lr = 0.00375
                elif epoch >= 200 and epoch < 250  :
                     lr = 0.002
                elif epoch >= 250 and epoch < 320 :
                     lr = 0.00125
                elif epoch >= 320 and epoch < 370:
                     lr = 0.000725
                elif epoch >= 370 and epoch < 400:
                     lr = 0.000525
                elif epoch >= 400 and epoch < 440:
                     lr = 0.000375
                elif epoch >= 440 and epoch < 470:
                     lr = 0.00025
                elif epoch >= 470 and epoch < 520:
                     lr = 0.000125
                elif epoch >= 520 and epoch < 560:
                     lr = 0.0000975
                elif epoch >= 560 and epoch < 590:
                     lr = 0.0000875
                elif epoch >= 590 and epoch < 630:
                     lr = 0.00008
                elif epoch >= 630 and epoch < 670:
                     lr = 0.000075
                elif epoch >= 670 and epoch < 700:
                     lr = 0.0000625
                elif epoch >= 700 and epoch < 730:
                     lr = 0.0000525
                elif epoch >= 730 and epoch < 770:
                     lr = 0.0000425
                elif epoch >= 770 and epoch < 800:
                     lr = 0.0000325
                elif epoch >= 800 and epoch < 830:
                     lr = 0.0000225
                elif epoch >= 830 and epoch < 865:
                     lr = 0.000015
                else:
                     lr = 0.0000125
               
                # lr = cosine_annealing(epoch=epoch,lr_min=self.config.lr_min,
                #                      lr_max=self.config.lr,n_epochs=self.config.n_epochs,
                #                     n_cycles=self.config.n_cycles)
                #lr = K.eval(self.model.optimizer.lr) 
                '''
                title = Markdown(f'# [LR-Cosine] Old Learning Rate : `{K.eval(self.model.optimizer.lr):.7f}` ==> New Learning Rate : `{lr:.7f}` ', self.config.info_style)
                console.print(title)
                K.set_value(self.model.optimizer.lr, lr)
                
            history = self.model.fit(input_data,{'policy': policy, 'value': value}, 
                                        epochs=1, batch_size=self.config.batch_size)
                        
            histories[epoch] = history.history
            
            if (epoch % self.config.chkp_frq == 0) :
                gc.collect()

            if (epoch % self.config.print_frq == 0):
                title = Markdown(f"## Validating...", style=self.config.succes_style)
                console.print(title)
                golois.getValidation(input_data, policy, value, end)
                val = self.model.evaluate(input_data,[policy, value], 
                                    verbose = self.config.verbose, batch_size=self.config.batch_size )
                title = Markdown(f"# Validation : {val}", style=self.config.succes_style)
                console.print(title)
                val_hist.append(val)
                self.model.save(f"cosine_inception_se_192_sw_{self.config.start_epoch}_to_{self.config.end_epoch}_{self.model_path}")       
        title = Markdown(f"## END of Training Saving Last [DGM]...", style=self.config.succes_style)
        console.print(title)
        histories['val_history'] = val_hist

        with open(f"{self.hist_path}_cosine_inception_se_192_sw_{self.config.end_epoch}", 'wb') as f_hist:
            pickle.dump(histories, f_hist)

        return histories




