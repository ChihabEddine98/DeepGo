# Reservation :
# oarsub -p "cluster='gemini'" -l host=1,walltime=40 -t deploy -t exotic -r '2021-12-03 20:00:00'
# This script is to run on top of Debian11-x64
# Use this to create the deb11 : 
# kadeploy3 -f $OAR_NODE_FILE -e debian11-x64-base -k
# Then : ssh root@gemini-2 

# To copy this scirpt into the server use : 
# scp deploy.sh lyon.g5k:/home/abenamara
# scp deploy.sh root@gemini-2:/root

apt-get update

echo 'Installing C++ stuff...'
# Install C++ compiler
apt install -y g++
apt install make

echo 'Installing Python[3.7.12] stuff...'
# Python Installation
apt-get -y build-dep libcurl4-openssl-dev
apt-get -y install libcurl4-openssl-dev
apt-get install libffi-dev
wget https://www.python.org/ftp/python/3.7.12/Python-3.7.12.tgz
tar -xvf Python-3.7.12.tgz
cd Python-3.7.12
./configure --enable-shared
make 
make test
make install
#apt install python3.7 -y
#update-alternatives --install /usr/bin/python python /usr/bin/python3.7 1
#apt install python-pip -y

echo 'Configuring Repository[DeepGo]...'
# Repository Installation
cd /root
apt install -y git 
git clone https://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@github.com/ChihabEddine98/DeepGo.git
cd DeepGo
pip3 install --ignore-installed -r requirements.txt

