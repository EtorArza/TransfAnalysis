#!/bin/bash

source scripts/array_to_string_functions.sh

TEST_RESULT_FOLDER_NAME="experiment_results/transfer_16_continuous_problems/resultsl"
EXPERIMENT_FOLDER_NAME="experiment_results/transfer_16_continuous_problems/controllers"

mkdir -p $TEST_RESULT_FOLDER_NAME
mkdir -p $EXPERIMENT_FOLDER_NAME


COMPILE_JOB_ID=`sbatch --parsable scripts/make_hip.sh --exclude=n[001-004,017-018]`

SRCDIR=`pwd`






SEED=2
NEAT_POPSIZE=500
SOLVER_POPSIZE=20
MAX_SOLVER_FE=20000
MAX_TRAIN_ITERATIONS=2
MAX_TRAIN_TIME=99999999999
FULL_MODEL="true"

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




TRAIN_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=NEAT_POPSIZE=${NEAT_POPSIZE},SOLVER_POPSIZE=${SOLVER_POPSIZE},MAX_SOLVER_FE=${MAX_SOLVER_FE},MAX_TRAIN_ITERATIONS=${MAX_TRAIN_ITERATIONS},MAX_TRAIN_TIME=${MAX_TRAIN_TIME},EXPERIMENT_FOLDER_NAME=${EXPERIMENT_FOLDER_NAME},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY},SEED_ARRAY=${SEED_ARRAY},FULL_MODEL_ARRAY=${FULL_MODEL_ARRAY},COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_INDEX_LIST_ARRAY},COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY=${COMMA_SEPARATED_PROBLEM_DIM_LIST_ARRAY}, --array=0-$i src/experiments/real/scripts/hip_train_array.sl`




