#!/bin/bash

EXPERIMENT_FOLDER_NAME="src/experiments/permus_multi/results/train_one_controller_for_each_problem_with_lots_of_resources"

#find src/experiments/permus/instances/comp_against_heuristics/instances/train/lop | xargs echo | sed 's/ /,/g'

source scripts/array_to_string_functions.sh

if false; then
echo "this part not executed"
# ... Code I want to skip here ...

fi

COMPILE_JOB_ID=`sbatch --parsable scripts/make_hip.sh`


SRCDIR=`pwd`



PROBLEM_TYPE_ARRAY=()
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=()
POPSIZE_ARRAY=()
CONTROLLER_NAME_PREFIX_ARRAY=()
EXPERIMENT_FOLDER_NAME_ARRAY=()
CONTROLLER_ARRAY=()
SEED_ARRAY=()
MAX_SOLVER_TIME_ARRAY=()
MAX_NEAT_TIME_ARRAY=()

POPSIZE=128
SEED=2


opt_times=("0.25" "1.0" "4.0")
neat_times=("108000" "432000" "1728000")

i=-1
for time_index in 2 1 0; do
    for PROBLEM_TYPE in "qap" "tsp" "pfsp" "lop"; do
        i=$((i+1))

        list_of_instances=`find src/experiments/permus/instances/comp_against_heuristics/instances/train/${PROBLEM_TYPE} -type f | xargs echo | sed 's/ /,/g'`

        CONTROLLER_NAME_PREFIX="multilarge_${PROBLEM_TYPE}"

        POPSIZE_ARRAY+=(${POPSIZE})
        SEED_ARRAY+=("${SEED}")

        OPTIMIZATION_TIME=${opt_times[time_index]}
        NEAT_TIME=${neat_times[time_index]}

        PROBLEM_TYPE_ARRAY+=("${PROBLEM_TYPE}")
        COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY+=("${list_of_instances}")
        replaced_with="_"
        CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}_${OPTIMIZATION_TIME//"."/$replaced_with}")
        EXPERIMENT_FOLDER_NAME_ARRAY+=("${SRCDIR}/${EXPERIMENT_FOLDER_NAME}")
        CONTROLLER_ARRAY+=("${EXPERIMENT_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        MAX_SOLVER_TIME_ARRAY+=("${OPTIMIZATION_TIME}")
        MAX_NEAT_TIME_ARRAY+=("${NEAT_TIME}")
    done
done





COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=$(to_list "${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY[@]}")
PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
POPSIZE_ARRAY=$(to_list "${POPSIZE_ARRAY[@]}")
CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
EXPERIMENT_FOLDER_NAME_ARRAY=$(to_list "${EXPERIMENT_FOLDER_NAME_ARRAY[@]}")
CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
MAX_SOLVER_TIME_ARRAY=$(to_list "${MAX_SOLVER_TIME_ARRAY[@]}")
MAX_NEAT_TIME_ARRAY=$(to_list "${MAX_NEAT_TIME_ARRAY[@]}")



# Replace all ocurrences of "," with "xyz_comma_xyz". 
# This is necesary because sbatch --export argument separes different variables with coma, 
# and if the values of the variables contain comas, it all gets mmessed up.
replaced_with="xyz_comma_xyz"
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY//","/$replaced_with}


TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=MAX_NEAT_TIME_ARRAY=${MAX_NEAT_TIME_ARRAY},PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY},EXPERIMENT_FOLDER_NAME_ARRAY=${EXPERIMENT_FOLDER_NAME_ARRAY},SEED_ARRAY=${SEED_ARRAY},POPSIZE_ARRAY=${POPSIZE_ARRAY},MAX_SOLVER_TIME_ARRAY=${MAX_SOLVER_TIME_ARRAY} --array=0-$i src/experiments/permus_multi/scripts/hip_train_multi_array_lots_of_resources.sl`



