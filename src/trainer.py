import pickle
import tensorflow as tf 
import gc

import golois

from utils import configs
from dataloader import DataHandler


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
        # Get Initial Data
        input_data , policy , value , end , groups = self.data_loader.get_data() 
        # Get Validation Data
        print('getting validation...')
        golois.getValidation(input_data, policy, value, end)

        histories = {} 
        for i in range (1, self.config.n_epochs + 1):
            print (f' Epoch [{i}]')
            golois.getBatch(input_data, policy, value, end, groups, i*self.config.n_samples)
            with tf.device(self.config.device):
                history = self.model.fit(input_data,{'policy': policy, 'value': value}, 
                                    epochs=1, batch_size=self.config.batch_size )
                histories[i] = history.history
            
            if (i % self.config.chkp_frq == 0) :
                gc.collect()

            if (i % self.config.print_frq == 0):
                print(f' Validating...')
                golois.getValidation(input_data, policy, value, end)
                val = self.model.evaluate(input_data,[policy, value], 
                                    verbose = self.config.verbose, batch_size=self.config.batch_size )
                print (f' Validation : {val}')
                self.model.save(self.model_path)
        
        with open(self.hist_path, 'wb') as f_hist:
            pickle.dump(histories, f_hist)

        return histories

