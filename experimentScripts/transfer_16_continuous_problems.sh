#!/bin/bash

source scripts/array_to_string_functions.sh

TEST_RESULT_FOLDER_NAME="experimentResults/transfer_16_continuous_problems/results"
EXPERIMENT_CONTROLLER_FOLDER_NAME="experimentResults/transfer_16_continuous_problems/controllers"

mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_CONTROLLER_FOLDER_NAME


COMPILE_JOB_ID=`sbatch --parsable scripts/make_hip.sh --exclude=n[001-004,017-018]`

SRCDIR=`pwd`






SEED=2
NEAT_POPSIZE=500
SOLVER_POPSIZE=20
MAX_SOLVER_FE=20000
MAX_TRAIN_ITERATIONS=200
MAX_TRAIN_TIME=99999999999
FULL_MODEL="false"
DIM=2 # some benchmark functions are only defined for DIM=2 


instance_list="[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]"
dim_list=`python -c "print([2]*16);"`
########################################################################


i=-1

# #region notExecuted
if false; then
echo "this part is not executed"
# ... Code I want to skip here ...


# ... end Code I want to skip 
fi
# #endregion 



# Leave one out of each type cross validation
j=-1
n_instances_of_each_type=4
for instance_index in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16; do
    i=$((i+1))
    j=$((j+1))


    CONTROLLER_NAME_PREFIX_ARRAY+=("LeaveOutF_${instance_index}")
    SEED_ARRAY+=("${SEED}")
    
    # Leave one out means 3 instances of each type
    COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY+=(`python -c "a = ${instance_list}; b = ${j} - ${j} % ${n_instances_of_each_type}; a=a[b:b+${n_instances_of_each_type}];a.pop(${j} % ${n_instances_of_each_type}); a = [str(el) for el in a]; print('s_e_p'.join(a))"`)
    COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY+=(`python -c "a = ${dim_list}; b = ${j} - ${j} % ${n_instances_of_each_type}; a=a[b:b+${n_instances_of_each_type}];a.pop(${j} % ${n_instances_of_each_type}); a = [str(el) for el in a]; print('s_e_p'.join(a))"`)

    FULL_MODEL_ARRAY+=("${FULL_MODEL}")
done



# Train in one
j=-1
for instance_index in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16; do
    i=$((i+1))
    j=$((j+1))


    CONTROLLER_NAME_PREFIX_ARRAY+=("TrainOnlyInF_${instance_index}")
    SEED_ARRAY+=("${SEED}")

    COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY+=(`python -c "print(${instance_list}[${j}])"`)
    COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY+=(`python -c "print(${dim_list}[${j}])"`)
    FULL_MODEL_ARRAY+=("${FULL_MODEL}")
done



# train in all
for FULL_MODEL_WHICH in "${FULL_MODEL}"; do

i=$((i+1))
CONTROLLER_NAME_PREFIX_ARRAY+=("trained_with_all_problems_${FULL_MODEL_WHICH}")
SEED_ARRAY+=("${SEED}")
COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY+=(`python -c "a = ${instance_list}; a = [str(el) for el in a]; print('s_e_p'.join(a))"`)
COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY+=(`python -c "a = ${dim_list}; a = [str(el) for el in a]; print('s_e_p'.join(a))"`)
FULL_MODEL_ARRAY+=("${FULL_MODEL_WHICH}")

done





CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
FULL_MODEL_ARRAY=$(to_list "${FULL_MODEL_ARRAY[@]}")
COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=$(to_list "${COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY[@]}")
COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=$(to_list "${COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY[@]}")




TRAIN_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=NEAT_POPSIZE=${NEAT_POPSIZE},SOLVER_POPSIZE=${SOLVER_POPSIZE},MAX_SOLVER_FE=${MAX_SOLVER_FE},MAX_TRAIN_ITERATIONS=${MAX_TRAIN_ITERATIONS},MAX_TRAIN_TIME=${MAX_TRAIN_TIME},EXPERIMENT_CONTROLLER_FOLDER_NAME=${EXPERIMENT_CONTROLLER_FOLDER_NAME},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY},SEED_ARRAY=${SEED_ARRAY},FULL_MODEL_ARRAY=${FULL_MODEL_ARRAY},COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY},COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY}, --array=0-$i src/experiments/real/scripts/hip_train_array.sl`



############  TEST ###########

COMPUTE_RESPONSE="true"
SCORE_PATH="$TEST_RESULT_FOLDER_NAME/score.txt"
RESPONSE_PATH="$TEST_RESULT_FOLDER_NAME/response.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname ${SCORE_PATH})
N_REPS=1
N_EVALS=10000

TESTING_JOB_ID=""

CONTROLLER_ARRAY=()
SEED_ARRAY=()
PROBLEM_INDEX_ARRAY=()
PROBLEM_DIM_ARRAY=()
i=-1

# Leave one out
for INSTANCE_INDEX_TRAIN in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16;do
    for INSTANCE_INDEX_TEST in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16;do

        # skipIterationFlag=`python -c "a = ${INSTANCE_INDEX_TRAIN} == ${INSTANCE_INDEX_TEST}; a = str(a).lower(); print(a)"`

        # if [ $skipIterationFlag == true ]; then 
        #     continue; 
        # fi

        i=$((i+1))


        CONTROLLER_NAME_PREFIX="LeaveOutF_${INSTANCE_INDEX_TRAIN}"

        CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        SEED_ARRAY+=("${SEED}")
        PROBLEM_INDEX_ARRAY+=("${INSTANCE_INDEX_TRAIN}")
        PROBLEM_DIM_ARRAY+=("${DIM}")
        
    done
done


# Train in one
for INSTANCE_INDEX_TRAIN in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16;do
    for INSTANCE_INDEX_TEST in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16;do

        # skipIterationFlag=`python -c "a = ${INSTANCE_INDEX_TRAIN} == ${INSTANCE_INDEX_TEST}; a = str(a).lower(); print(a)"`

        # if [ $skipIterationFlag == true ]; then 
        #     continue; 
        # fi

        i=$((i+1))


        CONTROLLER_NAME_PREFIX="TrainOnlyInF_${INSTANCE_INDEX_TRAIN}"

        CONTROLLER_ARRAY+=("${EXPERIMENT_CONTROLLER_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        SEED_ARRAY+=("${SEED}")
        PROBLEM_INDEX_ARRAY+=("${INSTANCE_INDEX_TRAIN}")
        PROBLEM_DIM_ARRAY+=("${DIM}")
        
    done
done





CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
PROBLEM_INDEX_ARRAY=$(to_list "${PROBLEM_INDEX_ARRAY[@]}")
PROBLEM_DIM_ARRAY=$(to_list "${PROBLEM_DIM_ARRAY[@]}")




TEST_JOB_ID=`sbatch --parsable --dependency=afterok:${TRAIN_JOB_ID} --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},SEED_ARRAY=${SEED_ARRAY},PROBLEM_INDEX_ARRAY=${PROBLEM_INDEX_ARRAY},PROBLEM_DIM_ARRAY=${PROBLEM_DIM_ARRAY},SOLVER_POPSIZE=${SOLVER_POPSIZE},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS},MAX_SOLVER_FE=${MAX_SOLVER_FE},FULL_MODEL=${FULL_MODEL} --array=0-$i src/experiments/real/scripts/hip_test_array_in_one_of_16_problems.sl`



sbatch --dependency=afterok:${TEST_JOB_ID} --export=SCORE_PATH=${SCORE_PATH},RESPONSE_PATH=${RESPONSE_PATH},COMPUTE_RESPONSE=${COMPUTE_RESPONSE},TMP_RES_PATH=${TMP_RES_PATH} scripts/cat_result_files_to_exp_folder.sh

