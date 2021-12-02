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
def cosine_annealing(epoch, n_epochs, n_cycles, lr_max):
    epochs_per_cycle = floor(n_epochs/n_cycles)
    cos_inner = (pi * (epoch % epochs_per_cycle)) / (epochs_per_cycle)
    return lr_max / 2 * (cos(cos_inner) + 1)

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
        golois.getValidation(input_data, policy, value, end)

        histories = {} 
        for epoch in range (1, self.config.n_epochs + 1):
            title = Markdown(f"# ----- epoch [{epoch}/{self.config.n_epochs}] -----", style=self.config.info_style)
            console.print(title)
            #print (f' Epoch [{epoch}/{self.config.n_epochs}]')
            golois.getBatch(input_data, policy, value, end, groups, epoch*self.config.n_samples)
            #with tf.device(self.config.device):
            if self.config.annealing :
                lr = cosine_annealing(epoch, self.config.n_epochs, self.config.n_cycles, self.config.lr)
                title = Markdown(f'# [LR-Cosine] Old Learning Rate : {K.eval(self.model.optimizer.lr)} ===> New Learning Rate : {lr} ', self.config.info_style)
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
                self.model.save(self.model_path)
        
        title = Markdown(f"## END of Training Saving Last [DGM]...", style=self.config.succes_style)
        with open(self.hist_path, 'wb') as f_hist:
            pickle.dump(histories, f_hist)

        return histories




