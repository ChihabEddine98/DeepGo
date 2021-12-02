import os 
import tensorflow as tf

from rich.console import Console
from rich.markdown import Markdown


from models.DGV1.model_v1 import DGMV1
from trainer import Trainer
from utils import configs


if __name__ == '__main__':

    console = Console()
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
    strategy = tf.distribute.MirroredStrategy(configs.devices)

    title = Markdown(f"# Start training ON {len(configs.devices)}GPU's", style='bold yellow')
    console.print(title)

    # Build the model 
    with strategy.scope():
        dgm = DGMV1() 
        model = dgm.build_model()
        dgm.summary()

    # Get Trainer
    trainer = Trainer(dgm)

    # Train
    history = trainer.train()

