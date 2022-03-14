#!/bin/bash
mkdir -p "/workspace/scratch/jobs/earza/slurm_logs"
source scripts/array_to_string_functions.sh


EXPERIMENT_RESULT_FOLDER_NAME="/workspace/scratch/jobs/earza/${PWD##*/}"

echo "Results saved on: $EXPERIMENT_RESULT_FOLDER_NAME"


TEST_RESULT_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/transfer_16_continuous_problems/results"
EXPERIMENT_CONTROLLER_FOLDER_NAME="$EXPERIMENT_RESULT_FOLDER_NAME/experimentResults/transfer_16_continuous_problems/controllers"
LOG_DIR="$EXPERIMENT_RESULT_FOLDER_NAME/logs"
SCORE_PATH="$TEST_RESULT_FOLDER_NAME/score.txt"
RESPONSE_PATH="$TEST_RESULT_FOLDER_NAME/response.txt"
TMP_RES_PATH=$EXPERIMENT_RESULT_FOLDER_NAME/"tmp"/$(dirname ${SCORE_PATH})
GLOBAL_LOG="${LOG_DIR}/global_log.txt"

mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_CONTROLLER_FOLDER_NAME
mkdir -p $LOG_DIR
mkdir -p $TMP_RES_PATH

COMPILE_JOB_ID=`sbatch --parsable --exclude=n[001-004] --export=LOG_DIR=${LOG_DIR} scripts/make_hip.sh`

SRCDIR=`pwd`






NEAT_POPSIZE=1000
SOLVER_POPSIZE=10
MAX_SOLVER_FE=1000
MAX_TRAIN_ITERATIONS=5000
MAX_TRAIN_TIME=345600
FULL_MODEL="false"
DIM=20


instance_list="[1,2,3,4,5,6,7,8,9,10,11,12]"
dim_list=`python -c "print([${DIM}]*12);"`
########################################################################


i=-1


# # Leave one out of each type cross validation
# j=-1
# n_instances_of_each_type=4
# for instance_index in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16; do
#     i=$((i+1))
#     j=$((j+1))


#     CONTROLLER_NAME_PREFIX_ARRAY+=("LeaveOutF_${instance_index}")
#     SEED_ARRAY+=("${SEED}")
    
#     # Leave one out means 3 instances of each type
#     COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY+=(`python -c "a = ${instance_list}; b = ${j} - ${j} % ${n_instances_of_each_type}; a=a[b:b+${n_instances_of_each_type}];a.pop(${j} % ${n_instances_of_each_type}); a = [str(el) for el in a]; print('s_e_p'.join(a))"`)
#     COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY+=(`python -c "a = ${dim_list}; b = ${j} - ${j} % ${n_instances_of_each_type}; a=a[b:b+${n_instances_of_each_type}];a.pop(${j} % ${n_instances_of_each_type}); a = [str(el) for el in a]; print('s_e_p'.join(a))"`)

#     FULL_MODEL_ARRAY+=("${FULL_MODEL}")
# done



# Train in one
for train_seed in 2 3 4 5 6 7 8 9 10 11; do
    j=-1
    for instance_index in 1 2 3 4 5 6 7 8 9 10 11 12; do
        i=$((i+1))
        j=$((j+1))


        CONTROLLER_NAME_PREFIX_ARRAY+=("TrainOnlyInF_${instance_index}_seed${train_seed}")
        SEED_ARRAY+=("${train_seed}")

        COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY+=(`python -c "print(${instance_list}[${j}])"`)
        COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY+=(`python -c "print(${dim_list}[${j}])"`)
        FULL_MODEL_ARRAY+=("${FULL_MODEL}")
    done
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





CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
FULL_MODEL_ARRAY=$(to_list "${FULL_MODEL_ARRAY[@]}")
COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=$(to_list "${COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY[@]}")
COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=$(to_list "${COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY[@]}")




TRAIN_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=NEAT_POPSIZE=${NEAT_POPSIZE},SOLVER_POPSIZE=${SOLVER_POPSIZE},MAX_SOLVER_FE=${MAX_SOLVER_FE},MAX_TRAIN_ITERATIONS=${MAX_TRAIN_ITERATIONS},MAX_TRAIN_TIME=${MAX_TRAIN_TIME},SEED_ARRAY=${SEED_ARRAY},FULL_MODEL_ARRAY=${FULL_MODEL_ARRAY},COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY},COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY},EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR},GLOBAL_LOG=${GLOBAL_LOG},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY}, --array=0-$i src/experiments/real/scripts/hip_train_array_16_continuous.sl`



############  TEST ###########

COMPUTE_RESPONSE="true"
N_REPS=1
N_EVALS=10000
TESTING_JOB_ID=""

for train_seed in 2 3 4 5 6 7 8 9 10 11; do


    CONTROLLER_ARRAY=()
    SEED_ARRAY=()
    PROBLEM_INDEX_ARRAY=()
    PROBLEM_DIM_ARRAY=()
    i=-1

    # # Leave one out
    # for INSTANCE_INDEX_TRAIN in 1 2 3 4 5 6 7 8 9 10 11 12;do
    #     for INSTANCE_INDEX_TEST in 1 2 3 4 5 6 7 8 9 10 11 12;do

    #         # skipIterationFlag=`python -c "a = ${INSTANCE_INDEX_TRAIN} == ${INSTANCE_INDEX_TEST}; a = str(a).lower(); print(a)"`

    #         # if [ $skipIterationFlag == true ]; then 
    #         #     continue; 
    #         # fi

    #         i=$((i+1))


    #         CONTROLLER_NAME_PREFIX="LeaveOutF_${INSTANCE_INDEX_TRAIN}"

    #         CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
    #         SEED_ARRAY+=("${SEED}")
    #         PROBLEM_INDEX_ARRAY+=("${INSTANCE_INDEX_TEST}")
    #         PROBLEM_DIM_ARRAY+=("${DIM}")
            
    #     done
    # done


    # Train in one
    for INSTANCE_INDEX_TRAIN in 1 2 3 4 5 6 7 8 9 10 11 12;do
        for INSTANCE_INDEX_TEST in 1 2 3 4 5 6 7 8 9 10 11 12;do

            # skipIterationFlag=`python -c "a = ${INSTANCE_INDEX_TRAIN} == ${INSTANCE_INDEX_TEST}; a = str(a).lower(); print(a)"`

            # if [ $skipIterationFlag == true ]; then 
            #     continue; 
            # fi

            i=$((i+1))


            CONTROLLER_NAME_PREFIX="TrainOnlyInF_${INSTANCE_INDEX_TRAIN}_seed${train_seed}"

            CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
            SEED_ARRAY+=("2") # the same seed for testing, the seed changes only for the controller name.
            PROBLEM_INDEX_ARRAY+=("${INSTANCE_INDEX_TEST}")
            PROBLEM_DIM_ARRAY+=("${DIM}")
            
        done
    done





CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
PROBLEM_INDEX_ARRAY=$(to_list "${PROBLEM_INDEX_ARRAY[@]}")
PROBLEM_DIM_ARRAY=$(to_list "${PROBLEM_DIM_ARRAY[@]}")



# we need to launch with each seed independently, otherwise argument list too long error
TEST_JOB_ID="${TEST_JOB_ID}:`sbatch --parsable --dependency=afterok:${TRAIN_JOB_ID} --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},SEED_ARRAY=${SEED_ARRAY},PROBLEM_INDEX_ARRAY=${PROBLEM_INDEX_ARRAY},PROBLEM_DIM_ARRAY=${PROBLEM_DIM_ARRAY},SOLVER_POPSIZE=${SOLVER_POPSIZE},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS},MAX_SOLVER_FE=${MAX_SOLVER_FE},FULL_MODEL=${FULL_MODEL},EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},TEST_RESULT_FOLDER_NAME=${TEST_RESULT_FOLDER_NAME},LOG_DIR=${LOG_DIR},GLOBAL_LOG=${GLOBAL_LOG} --array=0-$i src/experiments/real/scripts/hip_test_array_in_one_of_16_problems.sl`"

done

echo ${TEST_JOB_ID}

# TEST_JOB_ID=":TEST_JOB_ID1:TEST_JOB_ID2:TEST_JOB_ID3"
sbatch --dependency=afterok${TEST_JOB_ID} --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh

