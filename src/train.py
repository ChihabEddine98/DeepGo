import os 
import tensorflow as tf
import tensorflow.keras as keras
from rich.console import Console
from rich.markdown import Markdown


from models.DGV2.model_v2 import DGMV2
from models.DGV2.model_v2_1 import DGMV2_1
from trainer import Trainer
from utils import configs


if __name__ == '__main__':

    console = Console()
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
    strategy = tf.distribute.MirroredStrategy(configs.devices)

    title = Markdown(f"# Start training ON {len(configs.devices)} GPU's", style=configs.info_style)
    console.print(title)

    # Build the model 
    with strategy.scope():
        dgm = DGMV2_1() 
        model = keras.models.load_model('model_478.h5')
        dgm.model = model
        dgm.summary()

    # Get Trainer
    trainer = Trainer(dgm)

    # Train
    history = trainer.train()

