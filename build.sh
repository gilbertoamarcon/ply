#!/bin/bash
src=src
bin=bin
g++ $src/main.cpp $src/pointCloud.cpp -Wall -O3 -o $bin/main -std=c++0x -lm -w -lGL -lGLU -lglut -lpthread -lyaml-cpp
$bin/main
