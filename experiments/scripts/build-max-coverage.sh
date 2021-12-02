#!/usr/bin/env bash

g++ -O3 -DNDEBUG -msse4.2 -Wall -Wno-unused-function -std=c++17 -I../../third-party/Empirical/include/ -I../../include/ -I../../third-party/ max_coverage.cc -o max-coverage -lstdc++fs
# g++ -g -Wall -Wno-unused-function -std=c++17 -I../../third-party/Empirical/include/ -I../../include/ -I../../third-party/ max_coverage.cc -o max-coverage -lstdc++fs