#!/bin/bash
src=src
bin=bin
h5c++ $src/main.cpp $src/pointCloud.cpp $src/H5-utils.cpp -Wall -O3 -o $bin/main -std=c++0x -lm -w -lGL -lGLU -lglut -lpthread -lsz -lz -ldl -lyaml-cpp
# $bin/main
 
