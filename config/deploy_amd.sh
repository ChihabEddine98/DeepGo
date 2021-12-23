echo 'Configuring TF-GPU in 🐳 docker...'
g5k-setup-docker --tmp 
alias drun='sudo docker run -it --network=host --device=/dev/kfd --device=/dev/dri --ipc=host --shm-size 16G --group-add video --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v /tmp/dockerx:/dockerx'
drun rocm/tensorflow:latest
# to exec lastest docker image docker exec -it `docker ps --format '{{.ID}}'`  /bin/bash
echo 'Configuring TF-GPU in 🐳 docker ✅'

alias drun='sudo docker build -it --network=host --device=/dev/kfd --device=/dev/dri --ipc=host --shm-size 16G --group-add video --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v /tmp/dockerx:/dockerx'
drun rocm/tensorflow:latest

echo 'Configuring Repository[DeepGo]...'
# Repository Installation
git config --global --replace-all user.email "ga_benamara@esi.dz"
git config --global user.username "ChihabEddine98"
git config --list
git clone https://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@github.com/ChihabEddine98/DeepGo.git
pip3 install rich pybind11
cd DeepGo/src
c++ -O3 -Wall -shared -std=c++11 -fsized-deallocation -fPIC `python3 -m pybind11 --includes` golois/golois.cpp -o golois$(python3-config --extension-suffix)
wget https://www.lamsade.dauphine.fr/~cazenave/games.1000000.data.zip
rm -rf games.1000000.data.zip
mv games.1000000.data games.data 
echo 'Configuring Repository[DeepGo] finished ✅ '