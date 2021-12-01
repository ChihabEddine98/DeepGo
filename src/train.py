import os 
from tensorflow.distribute import MirroredStrategy
from models.DGV2.model_v2 import DGM_ShuffleNet
from trainer import Trainer
from utils import configs


if __name__ == '__main__':

    print(f'start training...')
    
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
    strategy = MirroredStrategy(configs.devices)

    print(f'start training...')

    # Build the model 
    with strategy.scope():
        dgm = DGM_ShuffleNet() 
        model = dgm.build_model()
        dgm.summary()

    # Get Trainer
    trainer = Trainer(dgm)

    # Train
    history = trainer.train()

