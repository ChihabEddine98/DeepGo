c++ -O3 -Wall -shared -std=c++11 -fPIC `python3 -m pybind11 --includes` golois.cpp -o golois`python3-config --extension-suffix` -I /usr/include/python3.7m/
