import os 
import tensorflow as tf
import tensorflow.keras as keras
from rich.console import Console
from rich.markdown import Markdown


#from models.DGV2.model_v2 import DGMV2
from models.DGV2.model_v2_1 import DGMV2_1
from models.DGV4.model_v4 import DGMV4
from models.DGV4.model_v4_mnet import DGMV5
from models.DGV5.model_v5 import DGMV2
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
        dgm = DGMV2() 
        #model = keras.models.load_model('5LR_89_2_DGMV3.h5')
        model = dgm.build_model() 
        dgm.model = model
        dgm.summary()

    # Get Trainer
    trainer = Trainer(dgm)

    # Train
    history = trainer.train()

