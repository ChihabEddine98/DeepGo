#!/bin/bash

# make sure pybind11 is installed : `pip3 install pybind11`

c++ -O3 -Wall -shared -std=c++11 -undefined dynamic_lookup $(python3 -m pybind11 --includes) ../src/golois.cpp -o golois$(python3-config --extension-suffix)
