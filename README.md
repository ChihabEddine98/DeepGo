# DeepGo
An AI that plays the game of GO (A deep neural network for policy &amp; value network)

## Get Started 
Create the docker container ( Python(3.8) + TF(2.6.0) )
> **Lunch:**   docker build -t deep-go -f Dockerfile .

Now you need to run this container : 
> **Run :**   docker run -it deep-go /bin/bash


# TODO

- [x] Create Generic Model
- [x]  DGV0 -- ReLU/swish activation
- [ ] try out e-swish : [here](https://arxiv.org/pdf/1801.07145.pdf)
- [ ] try h-swish [here](https://paperswithcode.com/method/hard-swish)
- [ ] SSH - GRID 5000



