# imports
from numpy import zeros
from numpy.random import randint
from tensorflow.keras.utils import to_categorical
from config import config
# end imports

'''
    -------------------------------------------------------------------------------------------     
        DataHandler : in this class we handle all stuff related to data shaping and/or
                    data to tensors creations this returned data will be helpful for training
                    and exploited by golois.cpp util function as (getBatch and getValidation).
    -------------------------------------------------------------------------------------------
'''
class DataHandler(object):
    
    def __init__(self,n_samples=config.n_samples,dim=config.dim,n_moves=config.n_moves,n_planes=config.n_planes) -> None:
        super().__init__()
        self.n_samples = n_samples
        self.dim = dim
        self.n_moves = n_moves
        self.n_planes = n_planes
    
    def get_data(self):
        # Inputs
        input_data = randint(2, size=(self.n_samples, self.dim, self.dim, self.n_planes)).astype ('float32')

        # Outputs 
        policy = to_categorical(randint(self.n_moves, size=(self.n_samples,)))
        value =  randint(2, size=(self.n_samples,)).astype ('float32')

        # For Get_Batch & Get_Validation
        end = randint(2, size=(self.n_samples, self.dim, self.dim, 2)).astype ('float32')
        groups = zeros((self.n_samples, self.dim, self.dim, 1)).astype ('float32')

        return input_data , policy , value , end , groups