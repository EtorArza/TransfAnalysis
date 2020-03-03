#!/bin/bash

srun bash scripts/make_hip.sh



JOB_IDS_PROBLEMS=(0 0 0 0)


i=0
for PROBLEM_TYPE in "qap" "tsp" "lop" "pfsp"; do
    INSTANCES=src/experiments/permus/instances/${PROBLEM}/*
    NUM_INSTANCES=$( ls -1 $INSTANCES | wc -l)
    ID=sbatch --export=PROBLEM_TYPE=$PROBLEM_TYPE,INSTANCES=$INSTANCES,MAX_TIME_PSO=0.25,N_EVALS=10,SAMPLE_SIZE_UPDATE_BK=3000,DESTINATION_FOLDER=src/experiments/permus/results/4by4_permu_problems/all_controllers --array=0-$NUM_INSTANCES src/experiments/permus/scripts/exec_hipatia_train.sh 
    JOB_IDS_PROBLEMS[i]=$ID
    i=$((i + 1))
done


for instance in src/experiments/permus/instances/qap/cut_instances/*; do
    sbatch src/experiments/permus/scripts/exec_hipatia_train.sh "qap" "$instance" 0.25
done


