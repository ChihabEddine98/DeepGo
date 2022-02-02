# DeepGO (DGM) 🤖
**Author .**  [Abdelkader Chihab Benamara](https://github.com/ChihabEddine98/) 
Several two headed neural networks with ~ less **1M** parameters that performs pretty good on Go game , trained on 1M games dataset from the KataGo dataset.
I have used the [Grid5000](https://www.grid5000.fr) clusters and trained my models using **8x Nvidia Tesla V100 (32 GiB)** GPU's and **512GB** of RAM.
And my current best model has **``57.75%``** validation accuracy for the policy head.

## Getting Started 🛠
Create the docker container ( Python(3.9) + TF(2.7.0) )
> **Lunch:**   docker build -t deep-go -f Dockerfile .

see [requirements.txt](https://github.com/ChihabEddine98/DeepGo/blob/main/requirements.txt) for more details

Now you need to run this container : 
> **Run :**   docker run -it deep-go /bin/bash

**Note.** The MacOS version isn't working currently there will be updates soon !

Or more simply you can use the following 
[deployment](https://github.com/ChihabEddine98/DeepGo/blob/main/config/deploy.sh) file  through  : 
>   **sh** ./path_to_deploy.sh

**Note.** If you are using an AMD GPU you can switch the deploy file and use the second one. 

## Train your own models 🏋️‍♂️
Actually i managed to make it easier for any one using this project to build and train his own model , first just you need to specify your architecture by creating a new folder inside [./src/models](https://github.com/ChihabEddine98/DeepGo/tree/main/src/models) and **``DGVx``** for example and then you can specify your model body (input and outputs blocks are already fine 😁) .
Then you can change the **``self.name``** for your version so you could have some pretty logs , and then you are all ready to go.
Use the following command to run and train your powerful model !

```python
   python?3 ./path_to_train.py -gpu 8 -s 1 -e 2000 -b 1024
```
This command train the model **``DGVx``** on **8** GPU's for **2000** epochs and with a batch size **1024**.
Here are the details for options that you can use for training : 
```python
---------------------------------------------------------------
				Command Line Guide (args to use)
---------------------------------------------------------------
-h [–-help]	          :  help with command line
-gpu [–-num-gpu]      :  \# of GPUs to be used 
						 for training [default : 2]

-s [--start-epoch]    : the epoch to porsuit the training 
					    from [default : 1]

-e [--end-epoch]      : the last epoch of training [default : 500]

-b [--batch-size]     : the batch size used to
                        train model [default : 512]

-n [--num-samples]    : \# of samples in the 
                        get_batch per epoch [default : 25k]

-w [--show-warnings]  : the warnings show level [from 0 to 3]

-l [--load][PATH]     : load Model to continue 
                        training from [PATH]
``` 

**Note .** Logs are made with the beautiful library [**``rich``**](https://github.com/Textualize/rich) in order to get more readable results.

## Why not some charts ? 📊
Yes ! you can do it u simply need to recover the history of your models which is done automatically through the earlier command for training then you can simply call the [./src/utils](https://github.com/ChihabEddine98/DeepGo/blob/main/src/utils.py) function : 
```python
def train_plots(epochs,histories,styles)
-- styles can be of the format (color-style)
   Ex. mo means magenta and dots ...
```
## Further Details

For more details and examples on how to use this charts see [./src/train_dgm.ipynb](https://github.com/ChihabEddine98/DeepGo/blob/main/src/train_dgm.ipynb)
And to see my results during the training and testing of my models you can take a look on my paper **[DeepGo]** via this [link](https://github.com/ChihabEddine98/DeepGo/blob/main/report.pdf).

## Contibute 
If you encountered a problem or want to contibute , don't hesitate to open an issue and/or reach out on LinkedIn : [Chihab Benamara](https://www.linkedin.com/in/chihab-eddine-benamara-65b811155/)
