#!/bin/bash

source scripts/array_to_string_functions.sh


srun bash scripts/make_hip.sh

JOB_IDS_PROBLEMS=(0 0 0 0 0)
i=0
for POPSIZE in "128" "256" "512" "1024" "2048"; do
    INSTANCES="src/experiments/permus/instances/qap/popsize_instance/*.qap"
    PROBLEM_TYPE="qap"
    NUM_INSTANCES=$( ls -1 $INSTANCES | wc -l)
    NUM_INSTANCES=$(($NUM_INSTANCES - 1))
    JOB_IDS_PROBLEMS[i]=`sbatch --parsable --export=PROBLEM_TYPE=$PROBLEM_TYPE,POPSIZE=$POPSIZE,INSTANCES=$INSTANCES,MAX_TIME_PSO=0.25,DESTINATION_FOLDER=src/experiments/permus/results/popsize_exp/all_controllers/popsize${POPSIZE} --array=0-$NUM_INSTANCES src/experiments/permus/scripts/hip_train_array.sl`
    i=$((i + 1))
done


# JOB_IDS_PROBLEMS_2=(0 0 0 0 0)
# i=0
# for POPSIZE in "128" "256" "512" "1024" "2048"; do
#     INSTANCES="src/experiments/permus/instances/qap/popsize_instance/*.qap"
#     PROBLEM_TYPE="qap"
#     NUM_INSTANCES=$( ls -1 $INSTANCES | wc -l)
#     NUM_INSTANCES=$(($NUM_INSTANCES - 1))
#     JOB_IDS_PROBLEMS_2[i]=`sbatch --parsable --dependency=afterok:${JOB_IDS_PROBLEMS[0]}:${JOB_IDS_PROBLEMS[1]}:${JOB_IDS_PROBLEMS[2]}:${JOB_IDS_PROBLEMS[3]}:${JOB_IDS_PROBLEMS[4]} --export=POPSIZE=${POPOSIZE},CONTROLLERS=${CONTROLLERS},PROBLEMS=${PROBLEMS},INSTANCES=${INSTANCES},MAX_TIME_PSO=0.25,MEASURE_RESPONSES=false,TMP_RES_PATH=${TMP_RES_PATH} --array=0-$NUM_JOBS src/experiments/permus/scripts/hip_test_array.sl`
#     i=$((i + 1))
# done

