c++ -O3 -Wall -shared -std=c++11 -fsized-deallocation -fPIC `python3 -m pybind11 --includes` golois/golois.cpp -o golois$(python3-config --extension-suffix)
