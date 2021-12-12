################################################
# All Requirements : gcc , g++ , make , git 
################################################

################################################
# Reservation :
# oarsub -p "cluster='gemini'" -l host=1,walltime=40 -t deploy -t exotic -r '2021-12-03 20:00:00'
# oarsub -p "gpu_count > 1 AND gpu_model NOT LIKE '%Radeon%'" -l host=1,walltime=8 -t deploy -t exotic -r '2021-12-11 17:40:00'
# This script is to run on top of Debian11-x64
# Use this to create the deb11 : 
# kadeploy3 -f $OAR_NODE_FILE -e debian11-x64-base -k
# kadeploy3 -f $OAR_NODE_FILE -e ubuntu2004-x64-min -k 
# Then : ssh root@gemini-2 

# To copy this scirpt into the server use : 
# scp deploy.sh lyon.g5k:/home/abenamara
# scp deploy.sh root@gemini-2:/root
################################################


sudo apt-get update

echo 'Installing C++ stuff...'
# Install C++ compiler
apt install -y gcc g++
apt install make

#echo 'Installing Python[3.7.12] stuff...'
# Python Installation
#apt-get install -y libssl-dev libffi-dev zlib1g-dev libreadline-gplv2-dev libncursesw5-dev \
 #                  libsqlite3-dev tk-dev libgdbm-dev libc6-dev libbz2-dev libcurl4-openssl-dev
#wget https://www.python.org/ftp/python/3.7.12/Python-3.7.12.tgz
#tar -xvf Python-3.7.12.tgz
#cd Python-3.7.12
#./configure --enable-shared
#make && make test && make install

#################### For Debian11 ##############################
#apt-get -y build-dep libcurl4-openssl-dev
#apt-get -y install libcurl4-openssl-dev
#apt-get install libffi-dev
#apt install python3.7 -y
#update-alternatives --install /usr/bin/python python /usr/bin/python3.7 1
#apt install python-pip -y
################################################


echo 'Configuring Repository[DeepGo]...'
# Repository Installation
cd /root
apt install -y git 
git clone https://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@github.com/ChihabEddine98/DeepGo.git
cd DeepGo
pip3 install --ignore-installed -r requirements.txt

apt install -y python3-pip

echo "Configuring GPU's..."
apt install -y software-properties-common
# Add NVIDIA package repositories
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/cuda-ubuntu1804.pin
sudo mv cuda-ubuntu1804.pin /etc/apt/preferences.d/cuda-repository-pin-600
sudo apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/7fa2af80.pub
sudo add-apt-repository "deb https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/ /"
sudo apt-get update

wget http://developer.download.nvidia.com/compute/machine-learning/repos/ubuntu1804/x86_64/nvidia-machine-learning-repo-ubuntu1804_1.0.0-1_amd64.deb

sudo apt install -y ./nvidia-machine-learning-repo-ubuntu1804_1.0.0-1_amd64.deb
sudo apt-get update

# Install NVIDIA driver
sudo apt-get install -y --no-install-recommends nvidia-driver-450
# Reboot. Check that GPUs are visible using the command: nvidia-smi

wget https://developer.download.nvidia.com/compute/machine-learning/repos/ubuntu1804/x86_64/libnvinfer7_7.1.3-1+cuda11.0_amd64.deb
sudo apt install -y ./libnvinfer7_7.1.3-1+cuda11.0_amd64.deb
sudo apt-get update

# Install development and runtime libraries (~4GB)
sudo apt-get install -y --no-install-recommends \
    cuda-11-0 \
    libcudnn8=8.0.4.30-1+cuda11.0  \
    libcudnn8-dev=8.0.4.30-1+cuda11.0


# Install TensorRT. Requires that libcudnn8 is installed above.
sudo apt-get install -y --no-install-recommends libnvinfer7=7.1.3-1+cuda11.0 \
    libnvinfer-dev=7.1.3-1+cuda11.0 \
    libnvinfer-plugin7=7.1.3-1+cuda11.0
---------------------------------------------
wget https://download.nvidia.com/XFree86/Linux-x86_64/450.51/NVIDIA-Linux-x86_64-450.51.run
sudo apt-get install -y linux-headers-`uname -r`
sh NVIDIA-Linux-x86_64-450.51.run -s --no-install-compat32-libs




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
wget https://www.lamsade.dauphine.fr/~cazenave/games.1000000.data.zip
git clone https://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@github.com/ChihabEddine98/DeepGo.git
cd DeepGo
pip3 install --ignore-installed -r requirements.txt
cd src
c++ -O3 -Wall -shared -std=c++11 -fsized-deallocation -fPIC `python3 -m pybind11 --includes` golois/golois.cpp -o golois$(python3-config --extension-suffix)
exit 
scp games.1000000.data.zip root@`head -1 $OAR_NODE_FILE`:/root
ssh root@`head -1 $OAR_NODE_FILE`
unzip games.1000000.data.zip
rm -rf games.1000000.data.zip
mv games.1000000.data DeepGo/src/games.data 
