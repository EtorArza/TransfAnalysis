#!/bin/bash

source scripts/array_to_string_functions.sh


COMPILE_JOB_ID=`sbatch --parsable scripts/make_hip.sh`

SRCDIR=`pwd`


OPTIMIZATION_TIME=0.1

######################## PERMUPROBLEMS TRANSFER #################################
EXPERIMENT_FOLDER_NAME="src/experiments/permus/results/4by4_permu_problems"


PROBLEM_TYPE_ARRAY=()
PROBLEM_PATH_ARRAY=()
POPSIZE_ARRAY=()
CONTROLLER_NAME_PREFIX_ARRAY=()
EXPERIMENT_FOLDER_NAME_ARRAY=()
SEED_ARRAY=()
MAX_SOLVER_TIME_ARRAY=()

i=-1
for PROBLEM_TYPE in "qap" "tsp" "pfsp" "lop"; do
    for PROBLEM_PATH in "src/experiments/permus/instances/transfer_permuproblems/${PROBLEM_TYPE}/"* ; do
        i=$((i+1))

        CONTROLLER_NAME_PREFIX=`basename ${PROBLEM_PATH}`

        PROBLEM_TYPE_ARRAY+=("${PROBLEM_TYPE}")
        PROBLEM_PATH_ARRAY+=("${PROBLEM_PATH}")
        POPSIZE_ARRAY+=("128")
        CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}")
        EXPERIMENT_FOLDER_NAME_ARRAY+=("${SRCDIR}/${EXPERIMENT_FOLDER_NAME}")
        SEED_ARRAY+=("2")
        MAX_SOLVER_TIME_ARRAY+=("${OPTIMIZATION_TIME}")
    done
done




PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")
POPSIZE_ARRAY=$(to_list "${POPSIZE_ARRAY[@]}")
CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
EXPERIMENT_FOLDER_NAME_ARRAY=$(to_list "${EXPERIMENT_FOLDER_NAME_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
MAX_SOLVER_TIME_ARRAY=$(to_list "${MAX_SOLVER_TIME_ARRAY[@]}")



TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=PROBLEM_TYPE_ARRAY=$PROBLEM_TYPE_ARRAY,PROBLEM_PATH_ARRAY=$PROBLEM_PATH_ARRAY,CONTROLLER_NAME_PREFIX_ARRAY=$CONTROLLER_NAME_PREFIX_ARRAY,EXPERIMENT_FOLDER_NAME_ARRAY=$EXPERIMENT_FOLDER_NAME_ARRAY,SEED_ARRAY=$SEED_ARRAY,POPSIZE_ARRAY=$POPSIZE_ARRAY,MAX_SOLVER_TIME_ARRAY=$MAX_SOLVER_TIME_ARRAY --array=0-$i src/experiments/permus/scripts/hip_train_array.sl`





MEASURE_RESPONSES="true"
SCORE_PATH="src/experiments/permus/results/4by4_permu_problems/result_score_transfer_permuproblem.txt"
RESPONSE_PATH="src/experiments/permus/results/4by4_permu_problems/result_response_transfer_permuproblem.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname ${SCORE_PATH})
N_REPS=1
N_EVALS=10000


TESTING_JOB_ID=""
for PROBLEM_TYPE_TRAIN in "qap" "tsp" "pfsp" "lop"; do
    CONTROLLER_ARRAY=()
    PROBLEM_TYPE_ARRAY=()
    PROBLEM_PATH_ARRAY=()
    MAX_SOLVER_TIME_ARRAY=()

    i=-1
    for PROBLEM_PATH_TRAIN in "src/experiments/permus/instances/transfer_permuproblems/${PROBLEM_TYPE_TRAIN}/"*; do
        for PROBLEM_TYPE_TEST in "qap" "tsp" "pfsp" "lop"; do
            for PROBLEM_PATH_TEST in "src/experiments/permus/instances/transfer_permuproblems/${PROBLEM_TYPE_TEST}/"*; do

                # # # we need every iteration to normalize transferability !!!
                # # Skip if training and testing instance is the same. i++ comes later, since this case is not added to experimentation
                # if [ "$PROBLEM_PATH_TRAIN" == "$PROBLEM_PATH_TEST" ]; then
                #     continue
                # fi

                i=$((i+1))

                CONTROLLER_NAME_PREFIX=`basename ${PROBLEM_PATH_TRAIN}`


                CONTROLLER_ARRAY+=("${EXPERIMENT_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
                PROBLEM_TYPE_ARRAY+=("${PROBLEM_TYPE_TEST}")
                PROBLEM_PATH_ARRAY+=("${PROBLEM_PATH_TEST}")
                MAX_SOLVER_TIME_ARRAY+=("${OPTIMIZATION_TIME}")
            done
        done
    done
    CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
    PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
    PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")
    MAX_SOLVER_TIME_ARRAY=$(to_list "${MAX_SOLVER_TIME_ARRAY[@]}")


    TESTING_JOB_ID=$TESTING_JOB_ID:`sbatch --dependency=afterok:${TRAINING_JOB_ID} --parsable  --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},PROBLEM_PATH_ARRAY=${PROBLEM_PATH_ARRAY},MAX_SOLVER_TIME_ARRAY=${MAX_SOLVER_TIME_ARRAY},MEASURE_RESPONSES=${MEASURE_RESPONSES},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS} --array=0-$i src/experiments/permus/scripts/hip_test_array.sl`

done



sbatch --dependency=afterok$TESTING_JOB_ID --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},MEASURE_RESPONSES=${MEASURE_RESPONSES},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh





######################### QAP TRANSFER ##########################
EXPERIMENT_FOLDER_NAME="src/experiments/permus/results/transfer_qap_with_cut_instances"



PROBLEM_TYPE_ARRAY=()
PROBLEM_PATH_ARRAY=()
POPSIZE_ARRAY=()
CONTROLLER_NAME_PREFIX_ARRAY=()
EXPERIMENT_FOLDER_NAME_ARRAY=()
CONTROLLER_ARRAY=()
SEED_ARRAY=()
MAX_SOLVER_TIME_ARRAY=()

i=-1
for PROBLEM_PATH in "src/experiments/permus/instances/transfer_qap_cut_instances/"*; do
    i=$((i+1))

    CONTROLLER_NAME_PREFIX=`basename ${PROBLEM_PATH}`

    PROBLEM_TYPE_ARRAY+=("qap")
    PROBLEM_PATH_ARRAY+=("${PROBLEM_PATH}")
    POPSIZE_ARRAY+=("128")
    CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}")
    EXPERIMENT_FOLDER_NAME_ARRAY+=("${SRCDIR}/${EXPERIMENT_FOLDER_NAME}")
    CONTROLLER_ARRAY+=("${EXPERIMENT_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
    SEED_ARRAY+=("2")
    MAX_SOLVER_TIME_ARRAY+=("${OPTIMIZATION_TIME}")
done





PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")
POPSIZE_ARRAY=$(to_list "${POPSIZE_ARRAY[@]}")
CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
EXPERIMENT_FOLDER_NAME_ARRAY=$(to_list "${EXPERIMENT_FOLDER_NAME_ARRAY[@]}")
CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
MAX_SOLVER_TIME_ARRAY=$(to_list "${MAX_SOLVER_TIME_ARRAY[@]}")


TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID}:${TRAINING_JOB_ID} --export=PROBLEM_TYPE_ARRAY=$PROBLEM_TYPE_ARRAY,PROBLEM_PATH_ARRAY=$PROBLEM_PATH_ARRAY,CONTROLLER_NAME_PREFIX_ARRAY=$CONTROLLER_NAME_PREFIX_ARRAY,EXPERIMENT_FOLDER_NAME_ARRAY=$EXPERIMENT_FOLDER_NAME_ARRAY,SEED_ARRAY=$SEED_ARRAY,POPSIZE_ARRAY=$POPSIZE_ARRAY,MAX_SOLVER_TIME_ARRAY=$MAX_SOLVER_TIME_ARRAY --array=0-$i src/experiments/permus/scripts/hip_train_array.sl`





MEASURE_RESPONSES="true"
SCORE_PATH="src/experiments/permus/results/transfer_qap_with_cut_instances/result_score_transfer_qap.txt"
RESPONSE_PATH="src/experiments/permus/results/transfer_qap_with_cut_instances/result_response_transfer_qap.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname ${SCORE_PATH})
N_REPS=1
N_EVALS=10000


CONTROLLER_ARRAY=()
PROBLEM_TYPE_ARRAY=()
PROBLEM_PATH_ARRAY=()
MAX_SOLVER_TIME_ARRAY=()

i=-1
for PROBLEM_PATH_TRAIN in "src/experiments/permus/instances/transfer_qap_cut_instances/"*; do
    for PROBLEM_PATH_TEST in "src/experiments/permus/instances/transfer_qap_cut_instances/"*; do

        # # # we need every iteration to normalize transferability !!!
        # # Skip if training and testing instance is the same. i++ comes later, since this case is not added to experimentation
        # if [ "$PROBLEM_PATH_TRAIN" == "$PROBLEM_PATH_TEST" ]; then
        #     continue
        # fi

        i=$((i+1))

        CONTROLLER_NAME_PREFIX=`basename ${PROBLEM_PATH_TRAIN}`

        CONTROLLER_ARRAY+=("${EXPERIMENT_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        PROBLEM_TYPE_ARRAY+=("qap")
        PROBLEM_PATH_ARRAY+=("${PROBLEM_PATH_TEST}")
        MAX_SOLVER_TIME_ARRAY+=("${OPTIMIZATION_TIME}")
    done
done

CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")
MAX_SOLVER_TIME_ARRAY=$(to_list "${MAX_SOLVER_TIME_ARRAY[@]}")


TESTING_JOB_ID=`sbatch --dependency=afterok:${TRAINING_JOB_ID} --parsable --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},PROBLEM_PATH_ARRAY=${PROBLEM_PATH_ARRAY},MAX_SOLVER_TIME_ARRAY=${MAX_SOLVER_TIME_ARRAY},MEASURE_RESPONSES=${MEASURE_RESPONSES},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS} --array=0-$i src/experiments/permus/scripts/hip_test_array.sl`





sbatch --dependency=afterok:$TESTING_JOB_ID --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},MEASURE_RESPONSES=${MEASURE_RESPONSES},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh

