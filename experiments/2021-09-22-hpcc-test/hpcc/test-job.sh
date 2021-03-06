#!/bin/bash
# The interpreter used to execute the script

#SBATCH --job-name=test_job
#SBATCH --mail-type=FAIL
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --mem=1000m
#SBATCH --time=00:01:00
#SBATCH --account=zamanlh0
#SBATCH --array=1-10

# The application(s) to execute along with its input arguments and options:

# -- Load whatever modules/software you need to run your job! --

# -- I like to define helpful variables up top --
OUTPUT_DIR=/scratch/zamanlh_root/zamanlh0/lalejini
CONFIG_DIR=/home/lalejini
JOB_SEED_OFFSET=100
SEED=$((JOB_SEED_OFFSET + SLURM_ARRAY_TASK_ID - 1))
RUN_DIR=${OUTPUT_DIR}/TEST-JOB-${SEED}

mkdir -p ${RUN_DIR}
cp ${CONFIG_DIR}/test.txt ${RUN_DIR}
cd ${RUN_DIR}
cat test.txt > log.txt
