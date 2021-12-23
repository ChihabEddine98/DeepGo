#######################################################################
# Total time to do this : ~  20 min
#######################################################################


# heree yoohoooo ! 
# To get Nvidia only gpu ! 
# oarsub -p "gpu_count > 0 AND gpu_model NOT LIKE 'Radeon%'" -l host=1,walltime=4 -t deploy -t exotic -I
# Use this to create the deb11 :
# Take about ~ 7 min 
# kadeploy3 -f $OAR_NODE_FILE -e debian11-x64-std -k
# Then : ssh root@`head -1 $OAR_NODE_FILE`
# See this link : https://stackoverflow.com/questions/66977227/could-not-load-dynamic-library-libcudnn-so-8-when-running-tensorflow-on-ubun
echo "Configuring GPU's..."
apt install -y software-properties-common
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-ubuntu2004.pin
sudo mv cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600
sudo apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/7fa2af80.pub
sudo add-apt-repository "deb https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/ /"
sudo apt-get update
sudo apt-get install libcudnn8=8.1.0.*-1+cuda11.2
sudo apt-get install libcudnn8-dev=8.1.0.*-1+cuda11.2
# Take about ~ 2 min 
#sudo apt-get install libcudnn8=${cudnn_version}-1+${cuda_version} # cudnn_version  = 8.1.0.* , cuda_version cuda11.2
#sudo apt-get install libcudnn8-dev=${cudnn_version}-1+${cuda_version}

# Check GPU with TF
# from tensorflow.python.client import device_lib 
# print(device_lib.list_local_devices())

echo 'Configuring Repository[DeepGo]...'
# Repository Installation
git config --global --replace-all user.email "ga_benamara@esi.dz"
git config --global user.username "ChihabEddine98"
git config --list
# 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
git clone https://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@github.com/ChihabEddine98/DeepGo.git
cd DeepGo
pip3 install --ignore-installed -r requirements.txt
cd src
c++ -O3 -Wall -shared -std=c++11 -fsized-deallocation -fPIC `python3 -m pybind11 --includes` golois/golois.cpp -o golois$(python3-config --extension-suffix)
echo 'Getting Games from [LAMSADE]...'
wget https://www.lamsade.dauphine.fr/~cazenave/games.1000000.data.zip
unzip games.1000000.data.zip
rm -rf games.1000000.data.zip
mv games.1000000.data DeepGo/src/games.data 
