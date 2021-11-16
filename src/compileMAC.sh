#!/bin/bash

# make sure pybind11 is installed : `pip3 install pybind11`

clang++ -v -O3 -Wall -shared -std=c++11 -fsized-deallocation  $(python3 -m pybind11 --includes) ./golois.cpp -o golois
