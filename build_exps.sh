#!/bin/bash


make clean

# Build single-threaded directed evolution model
echo "Compiling single-threaded directed digital evolution..."
export PROJECT=directed-digital-evolution
export MAIN_CPP=source/native.cpp
export THREADING=-DDIRDEVO_SINGLE_THREAD
make native
echo "...Done."

# Build single-threaded gp
echo "Compiling single-threaded genetic programming experiment..."
export PROJECT=avidagp-ec
export MAIN_CPP=source/native-ec.cpp
export THREADING=-DDIRDEVO_SINGLE_THREAD
make native
echo "...Done."