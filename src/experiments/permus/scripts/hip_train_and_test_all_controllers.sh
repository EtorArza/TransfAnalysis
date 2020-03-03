#!/bin/bash

srun bash scripts/make_hip.sh



JOB_IDS_PROBLEMS=(0 0 0 0)


i=0
for PROBLEM_TYPE in "qap" "tsp" "lop" "pfsp"; do
    INSTANCES="src/experiments/permus/instances/${PROBLEM_TYPE}/*.${PROBLEM_TYPE}"
    NUM_INSTANCES=$( ls -1 $INSTANCES | wc -l)
    ID=`sbatch --export=PROBLEM_TYPE=$PROBLEM_TYPE,INSTANCES=$INSTANCES,MAX_TIME_PSO=0.25,N_EVALS=10,SAMPLE_SIZE_UPDATE_BK=3000,DESTINATION_FOLDER=src/experiments/permus/results/4by4_permu_problems/all_controllers --array=0-$NUM_INSTANCES src/experiments/permus/scripts/hip_train_array.sl`
    JOB_IDS_PROBLEMS[i]=${ID##* }
    i=$((i + 1))
done




INSTANCES="src/experiments/permus/instances/qap/cut_instances/*"
NUM_INSTANCES=$( ls -1 $INSTANCES | wc -l)
ID=`sbatch --export=PROBLEM_TYPE="qap",INSTANCES=$INSTANCES,MAX_TIME_PSO=0.25,N_EVALS=10,SAMPLE_SIZE_UPDATE_BK=3000,DESTINATION_FOLDER=src/experiments/permus/results/transfer_qap_with_cut_instances/all_controllers --array=0-$NUM_INSTANCES src/experiments/permus/scripts/hip_train_array.sl`
ID_2=${ID##* }




sbatch --dependency=afterok:${JOB_IDS_PROBLEMS[0]}:${JOB_IDS_PROBLEMS[1]}:${JOB_IDS_PROBLEMS[2]}:${JOB_IDS_PROBLEMS[3]}:$ID_2 src/experiments/permus/scripts/hip_test_all_controllers.sh





