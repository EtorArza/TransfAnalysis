#!/bin/bash
#PBS -q bcam
#PBS -l nodes=1:ppn=52
#PBS -l mem=52gb
#PBS -l cput=1:00:00
#PBS -N train_controllers



export NPROCS=`wc -l < $PBS_NODEFILE`




prob=${problem_name}
ins=${instance_path}




module load intel/2017b



cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=false
CFLAGS=-fopenmp -std=c++11 -pthread

PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF

make

echo "-compiled-"

cat > tmp.ini <<EOF
; config file for train in hpc hipatia


[Global] 
mode = train


[NEAT]
MAX_TRAIN_TIME = 3000
POPSIZE = 520
THREADS = 52
N_EVALS = 3
N_REEVALS_TOP_5_PERCENT = 52
N_EVALS_TO_UPDATE_BK = 5200



SEARCH_TYPE = phased
DELETE_PREVIOUS_EXPERIMENT = true
SEED = 2
START_WITHOUT_HIDDEN = false



[Controller]
MAX_TIME_PSO = 0.5
POPSIZE = 20
TABU_LENGTH = 40

PROBLEM_TYPE = $1
PROBLEM_PATH = $2

EOF




date
./neat "tmp.ini" >$ OUTPUT_FILE
date






