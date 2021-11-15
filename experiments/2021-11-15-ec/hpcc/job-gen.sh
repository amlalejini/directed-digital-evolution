#!/usr/bin/env bash

REPLICATES=50
EXP_SLUG=2021-11-15-ec

SCRATCH_EXP_DIR=/mnt/scratch/lalejini/data/dirdevo
HOME_EXP_DIR=/mnt/home/lalejini/devo_ws/directed-digital-evolution/experiments

DATA_DIR=${SCRATCH_EXP_DIR}/${EXP_SLUG}
JOB_DIR=${SCRATCH_EXP_DIR}/${EXP_SLUG}/jobs
CONFIG_DIR=${HOME_EXP_DIR}/${EXP_SLUG}/hpcc/config

python3 gen-sub.py --data_dir ${DATA_DIR}  --config_dir ${CONFIG_DIR} --replicates ${REPLICATES} --job_dir ${JOB_DIR}