#!/bin/bash

source scripts/array_to_string_functions.sh

EXPERIMENT_RESULT_FOLDER_NAME="/workspace/scratch/jobs/earza/${PWD##*/}"

echo "Results saved on: $EXPERIMENT_RESULT_FOLDER_NAME"


TEST_RESULT_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/transfer_permus_problems/results"
EXPERIMENT_CONTROLLER_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/transfer_permus_problems/controllers"
LOG_DIR="$EXPERIMENT_RESULT_FOLDER_NAME/logs"
SCORE_PATH="$TEST_RESULT_FOLDER_NAME/score.txt"
RESPONSE_PATH="$TEST_RESULT_FOLDER_NAME/response.txt"
TMP_RES_PATH=$EXPERIMENT_RESULT_FOLDER_NAME/"tmp"/$(dirname ${SCORE_PATH})
GLOBAL_LOG="${LOG_DIR}/global_log.txt"
INSTANCES_PATH="$EXPERIMENT_RESULT_FOLDER_NAME/instances"

mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_CONTROLLER_FOLDER_NAME
mkdir -p $LOG_DIR
mkdir -p $TMP_RES_PATH

cp -f -r -v "src/experiments/permus/instances/" "$INSTANCES_PATH/" >> ${GLOBAL_LOG}


COMPILE_JOB_ID=`sbatch --parsable --exclude=n[001-004] --export=LOG_DIR=${LOG_DIR} scripts/make_hip.sh`

SRCDIR=`pwd`


SEED=2
NEAT_POPSIZE=500
SOLVER_POPSIZE=10
MAX_SOLVER_FE=1000
MAX_TRAIN_ITERATIONS=10000
MAX_TRAIN_TIME=345600


if false; then
echo "this part not executed"
# ... Code I want to skip here ...
fi



######################## PERMUPROBLEMS TRANSFER #################################
PROBLEM_TYPE_ARRAY=()
PROBLEM_PATH_ARRAY=()
CONTROLLER_NAME_PREFIX_ARRAY=()
SEED_ARRAY=()

i=-1
for PROBLEM_TYPE in "qap" "tsp" "pfsp" "lop"; do
    for PROBLEM_PATH in "$INSTANCES_PATH/transfer_permuproblems/${PROBLEM_TYPE}/"* ; do
        i=$((i+1))

        CONTROLLER_NAME_PREFIX=`basename ${PROBLEM_PATH}`

        PROBLEM_TYPE_ARRAY+=("${PROBLEM_TYPE}")
        COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY+=("${PROBLEM_PATH}")
        CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}")
        SEED_ARRAY+=("2")
    done
done




PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$(to_list "${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY[@]}")
CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")



TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=PROBLEM_TYPE_ARRAY=$PROBLEM_TYPE_ARRAY,COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY,SEED_ARRAY=$SEED_ARRAY,NEAT_POPSIZE=$NEAT_POPSIZE,MAX_SOLVER_FE=$MAX_SOLVER_FE,MAX_TRAIN_ITERATIONS=$MAX_TRAIN_ITERATIONS,MAX_TRAIN_TIME=$MAX_TRAIN_TIME,EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR},GLOBAL_LOG=${GLOBAL_LOG},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY} --array=0-$i src/experiments/permus_multi/scripts/hip_train_multi_array.sl`


############  TEST ###########

COMPUTE_RESPONSE="true"
N_REPS=1
N_EVALS=10000


TESTING_JOB_ID=""
for PROBLEM_TYPE_TRAIN in "qap" "tsp" "pfsp" "lop"; do
    CONTROLLER_ARRAY=()
    PROBLEM_TYPE_ARRAY=()
    PROBLEM_PATH_ARRAY=()

    i=-1
    for PROBLEM_PATH_TRAIN in "$INSTANCES_PATH/transfer_permuproblems/${PROBLEM_TYPE_TRAIN}/"*; do
        for PROBLEM_TYPE_TEST in "qap" "tsp" "pfsp" "lop"; do
            for PROBLEM_PATH_TEST in "$INSTANCES_PATH/transfer_permuproblems/${PROBLEM_TYPE_TEST}/"*; do

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
            done
        done
    done
    CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
    PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
    PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")


    TESTING_JOB_ID=$TESTING_JOB_ID:`sbatch --dependency=afterok:${TRAINING_JOB_ID} --parsable  --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},PROBLEM_PATH_ARRAY=${PROBLEM_PATH_ARRAY},MAX_SOLVER_FE=${MAX_SOLVER_FE},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR},GLOBAL_LOG=${GLOBAL_LOG} --array=0-$i src/experiments/permus/scripts/hip_test_array.sl`

done



sbatch --dependency=afterok$TESTING_JOB_ID --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh




######################### QAP TRANSFER ##########################
TEST_RESULT_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/transfer_permus_qap/results"
EXPERIMENT_CONTROLLER_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/transfer_permus_qap/controllers"
LOG_DIR="$EXPERIMENT_RESULT_FOLDER_NAME/logs"
SCORE_PATH="$TEST_RESULT_FOLDER_NAME/score.txt"
RESPONSE_PATH="$TEST_RESULT_FOLDER_NAME/response.txt"
TMP_RES_PATH=$EXPERIMENT_RESULT_FOLDER_NAME/"tmp"/$(dirname ${SCORE_PATH})
GLOBAL_LOG="${LOG_DIR}/global_log.txt"


mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_CONTROLLER_FOLDER_NAME




mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_CONTROLLER_FOLDER_NAME



PROBLEM_TYPE_ARRAY=()
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=()
CONTROLLER_NAME_PREFIX_ARRAY=()
CONTROLLER_ARRAY=()
SEED_ARRAY=()

i=-1
for PROBLEM_PATH in "$INSTANCES_PATH/transfer_qap_cut_instances/"*; do
    i=$((i+1))

    CONTROLLER_NAME_PREFIX=`basename ${PROBLEM_PATH}`

    PROBLEM_TYPE_ARRAY+=("qap")
    COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY+=("${PROBLEM_PATH}")
    CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}")
    CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
    SEED_ARRAY+=("2")
done





PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$(to_list "${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY[@]}")
CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")


TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=PROBLEM_TYPE_ARRAY=$PROBLEM_TYPE_ARRAY,COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY,SEED_ARRAY=$SEED_ARRAY,NEAT_POPSIZE=$NEAT_POPSIZE,MAX_SOLVER_FE=$MAX_SOLVER_FE,MAX_TRAIN_ITERATIONS=$MAX_TRAIN_ITERATIONS,MAX_TRAIN_TIME=$MAX_TRAIN_TIME,EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR},GLOBAL_LOG=${GLOBAL_LOG},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY} --array=0-$i src/experiments/permus_multi/scripts/hip_train_multi_array.sl`


############  TEST ###########

COMPUTE_RESPONSE="true"
N_REPS=1
N_EVALS=10000


CONTROLLER_ARRAY=()
PROBLEM_TYPE_ARRAY=()
PROBLEM_PATH_ARRAY=()

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
    done
done

CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")


TESTING_JOB_ID=`sbatch --dependency=afterok:${TRAINING_JOB_ID} --parsable --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},PROBLEM_PATH_ARRAY=${PROBLEM_PATH_ARRAY},MAX_SOLVER_FE=${MAX_SOLVER_FE},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR},GLOBAL_LOG=${GLOBAL_LOG} --array=0-$i src/experiments/permus/scripts/hip_test_array.sl`





sbatch --dependency=afterok:$TESTING_JOB_ID --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh

