



# Copy Conda from Grenoble to drac
Miniconda3-py37_4.10.3-Linux-ppc64le.sh root@drac-9:/root
ssh root@`head -1 $OAR_NODE_FILE`
sh -y Miniconda3-py37_4.10.3-Linux-ppc64le.sh
conda create -n 'py37' python=3.7
conda activate py37
conda config --prepend channels https://public.dhe.ibm.com/ibmdl/export/pub/software/server/ibm-ai/conda/
conda install tensorflow-gpu pybind11 ricb


git clone https://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@github.com/ChihabEddine98/DeepGo.git
cd DeepGo/src
c++ -O3 -Wall -shared -std=c++11 -fsized-deallocation -fPIC `python3 -m pybind11 --includes` golois/golois.cpp -o golois$(python3-config --extension-suffix)
echo 'Getting Games from [LAMSADE]...'
wget https://www.lamsade.dauphine.fr/~cazenave/games.1000000.data.zip
unzip games.1000000.data.zip
rm -rf games.1000000.data.zip
mv games.1000000.data games.data 