#!/bin/bash

# This script downloads the data for a given experiment from the specified OSF repository.

# Has osf project environment variable been set?
if [[ -z "$OSF_PROJECT" ]]; then
  echo "OSF_PROJECT not set"
  exit
fi

# Has experiment tag environment variable been set?
if [[ -z "$EXP_TAG" ]]; then
  echo "EXP_TAG not set"
  exit
fi

# Has project path environment variable been set?
if [[ -z "$PROJECT_PATH" ]]; then
  echo "PROJECT_PATH not set"
  exit
fi

osf -p ${OSF_PROJECT} fetch data/${EXP_TAG}-data.tar.gz ${PROJECT_PATH}/experiments/${EXP_TAG}/analysis/${EXP_TAG}-data.tar.gz
tar -xzf ${PROJECT_PATH}/experiments/${EXP_TAG}/analysis/${EXP_TAG}-data.tar.gz -C ${PROJECT_PATH}/experiments/${EXP_TAG}/analysis/
echo "downloaded ${EXP_TAG} data"
