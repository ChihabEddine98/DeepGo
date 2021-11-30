from models.DGV2.model_v2 import DGM_ShuffleNet
from trainer import Trainer


if __name__ == '__main__':

    print(f'start training...')
    
    # Build the model 
    dgm = DGM_ShuffleNet() 
    model = dgm.build_model()
    dgm.summary()

    # Get Trainer
    trainer = Trainer(dgm)

    # Train
    history = trainer.train()

