#!/bin/bash

source scripts/array_to_string_functions.sh

TEST_RESULT_FOLDER_NAME="experimentResults/transfer_permus_problems/results"
EXPERIMENT_CONTROLLER_FOLDER_NAME="experimentResults/transfer_permus_problems/controllers"

mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_CONTROLLER_FOLDER_NAME



COMPILE_JOB_ID=`sbatch --parsable scripts/make_hip.sh`

SRCDIR=`pwd`


MAX_SOLVER_FE=20000
NEAT_POPSIZE=500
MAX_TRAIN_ITERATIONS=2000


if false; then
echo "this part not executed"
# ... Code I want to skip here ...
fi



######################## PERMUPROBLEMS TRANSFER #################################
PROBLEM_TYPE_ARRAY=()
PROBLEM_PATH_ARRAY=()
CONTROLLER_NAME_PREFIX_ARRAY=()
SEED_ARRAY=()
MAX_SOLVER_FE_ARRAY=()

i=-1
for PROBLEM_TYPE in "qap" "tsp" "pfsp" "lop"; do
    for PROBLEM_PATH in "src/experiments/permus/instances/transfer_permuproblems/${PROBLEM_TYPE}/"* ; do
        i=$((i+1))

        CONTROLLER_NAME_PREFIX=`basename ${PROBLEM_PATH}`

        PROBLEM_TYPE_ARRAY+=("${PROBLEM_TYPE}")
        COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY+=("${PROBLEM_PATH}")
        CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}")
        SEED_ARRAY+=("2")
        MAX_SOLVER_FE_ARRAY+=("${MAX_SOLVER_FE}")
    done
done




PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$(to_list "${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY[@]}")
CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
MAX_SOLVER_FE_ARRAY=$(to_list "${MAX_SOLVER_FE_ARRAY[@]}")



TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=PROBLEM_TYPE_ARRAY=$PROBLEM_TYPE_ARRAY,COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY,CONTROLLER_NAME_PREFIX_ARRAY=$CONTROLLER_NAME_PREFIX_ARRAY,EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},SEED_ARRAY=$SEED_ARRAY,NEAT_POPSIZE=$NEAT_POPSIZE,MAX_SOLVER_FE_ARRAY=$MAX_SOLVER_FE_ARRAY,MAX_TRAIN_ITERATIONS=$MAX_TRAIN_ITERATIONS --array=0-$i src/experiments/permus_multi/scripts/hip_train_multi_array.sl`


############  TEST ###########

COMPUTE_RESPONSE="true"
SCORE_PATH="$TEST_RESULT_FOLDER_NAME/score.txt"
RESPONSE_PATH="$TEST_RESULT_FOLDER_NAME/response.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname ${SCORE_PATH})
N_REPS=1
N_EVALS=10000


TESTING_JOB_ID=""
for PROBLEM_TYPE_TRAIN in "qap" "tsp" "pfsp" "lop"; do
    CONTROLLER_ARRAY=()
    PROBLEM_TYPE_ARRAY=()
    PROBLEM_PATH_ARRAY=()
    MAX_SOLVER_FE_ARRAY=()

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


                CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
                PROBLEM_TYPE_ARRAY+=("${PROBLEM_TYPE_TEST}")
                PROBLEM_PATH_ARRAY+=("${PROBLEM_PATH_TEST}")
                MAX_SOLVER_FE_ARRAY+=("${MAX_SOLVER_FE}")
            done
        done
    done
    CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
    PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
    PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")
    MAX_SOLVER_FE_ARRAY=$(to_list "${MAX_SOLVER_FE_ARRAY[@]}")


    TESTING_JOB_ID=$TESTING_JOB_ID:`sbatch --dependency=afterok:${TRAINING_JOB_ID} --parsable  --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},PROBLEM_PATH_ARRAY=${PROBLEM_PATH_ARRAY},MAX_SOLVER_FE_ARRAY=${MAX_SOLVER_FE_ARRAY},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS} --array=0-$i src/experiments/permus/scripts/hip_test_array.sl`

done



sbatch --dependency=afterok$TESTING_JOB_ID --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh




######################### QAP TRANSFER ##########################
TEST_RESULT_FOLDER_NAME="experimentResults/transfer_permus_qap/results"
EXPERIMENT_CONTROLLER_FOLDER_NAME="experimentResults/transfer_permus_qap/controllers"

mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_CONTROLLER_FOLDER_NAME



PROBLEM_TYPE_ARRAY=()
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=()
CONTROLLER_NAME_PREFIX_ARRAY=()
CONTROLLER_ARRAY=()
SEED_ARRAY=()
MAX_SOLVER_FE_ARRAY=()

i=-1
for PROBLEM_PATH in "src/experiments/permus/instances/transfer_qap_cut_instances/"*; do
    i=$((i+1))

    CONTROLLER_NAME_PREFIX=`basename ${PROBLEM_PATH}`

    PROBLEM_TYPE_ARRAY+=("qap")
    COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY+=("${PROBLEM_PATH}")
    CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}")
    CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
    SEED_ARRAY+=("2")
    MAX_SOLVER_FE_ARRAY+=("${OPTIMIZATION_TIME}")
done





PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$(to_list "${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY[@]}")
CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
MAX_SOLVER_FE_ARRAY=$(to_list "${MAX_SOLVER_FE_ARRAY[@]}")


TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=PROBLEM_TYPE_ARRAY=$PROBLEM_TYPE_ARRAY,COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY,CONTROLLER_NAME_PREFIX_ARRAY=$CONTROLLER_NAME_PREFIX_ARRAY,EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},SEED_ARRAY=$SEED_ARRAY,NEAT_POPSIZE=$NEAT_POPSIZE,MAX_SOLVER_FE_ARRAY=$MAX_SOLVER_FE_ARRAY,MAX_TRAIN_ITERATIONS=$MAX_TRAIN_ITERATIONS --array=0-$i src/experiments/permus_multi/scripts/hip_train_multi_array.sl`


############  TEST ###########

COMPUTE_RESPONSE="true"
SCORE_PATH="$TEST_RESULT_FOLDER_NAME/score.txt"
RESPONSE_PATH="$TEST_RESULT_FOLDER_NAME/response.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname ${SCORE_PATH})
N_REPS=1
N_EVALS=10000


CONTROLLER_ARRAY=()
PROBLEM_TYPE_ARRAY=()
PROBLEM_PATH_ARRAY=()
MAX_SOLVER_FE_ARRAY=()

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

        CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        PROBLEM_TYPE_ARRAY+=("qap")
        PROBLEM_PATH_ARRAY+=("${PROBLEM_PATH_TEST}")
        MAX_SOLVER_FE_ARRAY+=("${OPTIMIZATION_TIME}")
    done
done

CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")
MAX_SOLVER_FE_ARRAY=$(to_list "${MAX_SOLVER_FE_ARRAY[@]}")


TESTING_JOB_ID=`sbatch --dependency=afterok:${TRAINING_JOB_ID} --parsable --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},PROBLEM_PATH_ARRAY=${PROBLEM_PATH_ARRAY},MAX_SOLVER_FE_ARRAY=${MAX_SOLVER_FE_ARRAY},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS} --array=0-$i src/experiments/permus/scripts/hip_test_array.sl`





sbatch --dependency=afterok:$TESTING_JOB_ID --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh

