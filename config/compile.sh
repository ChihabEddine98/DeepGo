c++ -O3 -Wall -shared -std=c++11 -fsized-deallocation `python3 -m pybind11 --includes` /app/src/golois.cpp -o golois
