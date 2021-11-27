import pickle
import tensorflow as tf 
import tensorflow.keras as keras
import golois

from config import config
from dataloader import DataHandler
from models.DGV1.model_v1 import DGM_MOBILENET




def train(model):
    histories = {} 
    for i in range (1, config.n_epochs + 1):
        print (f' Epoch [{i}]')
        golois.getBatch(input_data, policy, value, end, groups, i*config.n_samples)
        with tf.device('/device:GPU:0'):
            history = model.fit(input_data,{'policy': policy, 'value': value}, 
                                epochs=1, batch_size=config.batch_size )
            histories[i] = history.history
        if (i % 5 == 0):
            golois.getValidation(input_data, policy, value, end)
            val = model.evaluate(input_data,[policy, value], 
                                verbose = 0, batch_size=config.batch_size )
            print (f' Validation : {val}')

    model.save ('DGV1.h5')
    
    with open('DGV1_history', 'wb') as f_hist:
        pickle.dump(histories, f_hist)

    return histories

if __name__ == '__main__':
    
    # Get Initial Data
    data_loader = DataHandler()
    input_data , policy , value , end , groups = data_loader.get_data() 

    # Build the model 
    dgm_v0 = DGM_MOBILENET() 
    model = dgm_v0.build_model()
    #dgm_v0.plot_model()
    model.summary()

    print('getting validation...')
    golois.getValidation(input_data, policy, value, end)


    model.compile(optimizer=keras.optimizers.Adam(learning_rate=config.lr),
                loss={'policy': 'categorical_crossentropy', 'value': 'binary_crossentropy'},
                loss_weights={'policy' : config.policy_w, 'value' : config.value_w},
                metrics={'policy': 'categorical_accuracy', 'value': 'mse'})

    history = train(model)

