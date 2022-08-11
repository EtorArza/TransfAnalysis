#!/bin/bash
mkdir -p "/workspace/scratch/jobs/earza/slurm_logs"
source scripts/array_to_string_functions.sh


EXPERIMENT_RESULT_FOLDER_NAME="/workspace/scratch/jobs/earza/${PWD##*/}"

echo "Results saved on: $EXPERIMENT_RESULT_FOLDER_NAME"


TEST_RESULT_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/visualize_network_response_change/results"
EXPERIMENT_CONTROLLER_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/visualize_network_response_change/controllers"
LOG_DIR="$EXPERIMENT_RESULT_FOLDER_NAME/logs"
SCORE_PATH="$TEST_RESULT_FOLDER_NAME/score.txt"
RESPONSE_PATH="$TEST_RESULT_FOLDER_NAME/response.txt"
TMP_RES_PATH=$EXPERIMENT_RESULT_FOLDER_NAME/"tmp"/$(dirname ${SCORE_PATH})

mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_CONTROLLER_FOLDER_NAME
mkdir -p $LOG_DIR
mkdir -p $TMP_RES_PATH

COMPILE_JOB_ID=`sbatch --parsable --exclude=n[001-004] --export=LOG_DIR=${LOG_DIR} scripts/make_hip.sh`

SRCDIR=`pwd`






NEAT_POPSIZE=1000
MAX_TRAIN_ITERATIONS=1000
MAX_TRAIN_TIME=345600
FULL_MODEL="false"
DIM=20


SOLVER_POPSIZE_ARRAY=(8   32   128  16  16   16)
MAX_SOLVER_FE_ARRAY=(1600 1600 1600 400 1600 6400)



instance_list="[1,2,3,4,5,6,7,8,9,10,11,12]"
dim_list=`python -c "print([${DIM}]*12);"`
########################################################################




# Train in one
train_seed=2
for param_index in 0 1 2 3 4 5 6 7; do
    j=-1
    CONTROLLER_NAME_PREFIX_ARRAY=()
    SEED_ARRAY=()
    COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=()
    COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=()
    FULL_MODEL_ARRAY=()
    for instance_index in 1; do
        i=$((i+1))
        j=$((j+1))



        SOLVER_POPSIZE="${SOLVER_POPSIZE_ARRAY[param_index]}"
        MAX_SOLVER_FE="${MAX_SOLVER_FE_ARRAY[param_index]}"


        CONTROLLER_NAME_PREFIX_ARRAY+=("TrainOnlyInF_${instance_index}_seed${train_seed}_MAXSOLVERFE_${MAX_SOLVER_FE}_SOLVERPOPSIZE_${SOLVER_POPSIZE}")
        SEED_ARRAY+=("${train_seed}")
        COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY+=(`python -c "print(${instance_list}[${j}])"`)
        COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY+=(`python -c "print(${dim_list}[${j}])"`)
        FULL_MODEL_ARRAY+=("${FULL_MODEL}")
    done




    CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
    SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
    FULL_MODEL_ARRAY=$(to_list "${FULL_MODEL_ARRAY[@]}")
    COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=$(to_list "${COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY[@]}")
    COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=$(to_list "${COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY[@]}")



    TRAIN_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=NEAT_POPSIZE=${NEAT_POPSIZE},SOLVER_POPSIZE=${SOLVER_POPSIZE},MAX_SOLVER_FE=${MAX_SOLVER_FE},MAX_TRAIN_ITERATIONS=${MAX_TRAIN_ITERATIONS},MAX_TRAIN_TIME=${MAX_TRAIN_TIME},SEED_ARRAY=${SEED_ARRAY},FULL_MODEL_ARRAY=${FULL_MODEL_ARRAY},COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY},COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY},EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY}, --array=0-$j src/experiments/real/scripts/hip_train_array_16_continuous.sl`

done


# # train in all
# for FULL_MODEL_WHICH in "${FULL_MODEL}"; do

# i=$((i+1))
# CONTROLLER_NAME_PREFIX_ARRAY+=("trained_with_all_problems_${FULL_MODEL_WHICH}")
# SEED_ARRAY+=("${SEED}")
# COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY+=(`python -c "a = ${instance_list}; a = [str(el) for el in a]; print('s_e_p'.join(a))"`)
# COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY+=(`python -c "a = ${dim_list}; a = [str(el) for el in a]; print('s_e_p'.join(a))"`)
# FULL_MODEL_ARRAY+=("${FULL_MODEL_WHICH}")

# done




