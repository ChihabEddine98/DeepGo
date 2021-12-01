import os 
import tensorflow as tf
from models.DGV1.model_v1 import DGMV1
from trainer import Trainer
from utils import configs


if __name__ == '__main__':

    print(f'start training...')
    
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
    strategy = tf.distribute.MirroredStrategy(configs.devices)

    print(f'start training...')

    # Build the model 
    with strategy.scope():
        dgm = DGMV1() 
        model = dgm.build_model()
        dgm.summary()

    # Get Trainer
    trainer = Trainer(dgm)

    # Train
    history = trainer.train()

