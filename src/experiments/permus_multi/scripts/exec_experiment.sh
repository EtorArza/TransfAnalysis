#!/bin/bash

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

POPSIZE=40
MAX_SOLVER_TIME="0.1"
SEED=2

echo "WARNING! toy parameters. Need the popsize experiment result to be able to execute this."


i=-1
for INSTANCE_TYPE_PAIR in "A|B" "A|C" "B|C";do
    for INSTANCE_INDEX in "1" "2" "3" "4" "5"; do
        i=$((i+1))

        list_to_array $INSTANCE_TYPE_PAIR
        INSTANCE_TYPE_PAIR_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")

        ins_path_0=`ls src/experiments/permus/instances/transfer_qap_cut_instances/${INSTANCE_TYPE_PAIR_ARRAY[0]}${INSTANCE_INDEX}*`
        ins_path_1=`ls src/experiments/permus/instances/transfer_qap_cut_instances/${INSTANCE_TYPE_PAIR_ARRAY[1]}${INSTANCE_INDEX}*`

        EXPERIMENT_FOLDER_NAME="src/experiments/permus_multi/results/qap_cut_multi_vs_mono"
        CONTROLLER_NAME_PREFIX="${INSTANCE_TYPE_PAIR_ARRAY[0]}${INSTANCE_TYPE_PAIR_ARRAY[1]}${INSTANCE_INDEX}"

        POPSIZE_ARRAY+=(${POPSIZE})
        SEED_ARRAY+=("${SEED}")

        PROBLEM_TYPE_ARRAY+=("qap")
        COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY+=("${ins_path_0},${ins_path_1}")
        CONTROLLER_NAME_PREFIX_ARRAY+=("${CONTROLLER_NAME_PREFIX}")
        EXPERIMENT_FOLDER_NAME_ARRAY+=("${SRCDIR}/${EXPERIMENT_FOLDER_NAME}")
        CONTROLLER_ARRAY+=("${EXPERIMENT_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        MAX_SOLVER_TIME_ARRAY+=("${MAX_SOLVER_TIME}")
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



# Replace all ocurrences of "," with "xyz_comma_xyz". 
# This is necesary because sbatch --export argument separes different variables with coma, 
# and if the values of the variables contain comas, it all gets mmessed up.
replaced_with="xyz_comma_xyz"
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY//","/$replaced_with}


#TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY},CONTROLLER_NAME_PREFIX_ARRAY=${CONTROLLER_NAME_PREFIX_ARRAY},EXPERIMENT_FOLDER_NAME_ARRAY=${EXPERIMENT_FOLDER_NAME_ARRAY},SEED_ARRAY=${SEED_ARRAY},POPSIZE_ARRAY=${POPSIZE_ARRAY},MAX_SOLVER_TIME_ARRAY=${MAX_SOLVER_TIME_ARRAY} --array=0-$i src/experiments/permus_multi/scripts/hip_train_array.sl`





MEASURE_RESPONSES="true"
EXPERIMENT_FOLDER_NAME="src/experiments/permus_multi/results/qap_cut_multi_vs_mono"
SCORE_PATH="src/experiments/permus_multi/results/qap_cut_multi_vs_mono/score_multi_instance_cut_qap.txt"
RESPONSE_PATH="src/experiments/permus_multi/results/qap_cut_multi_vs_mono/response_multi_instance_cut_qap.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname ${SCORE_PATH})
N_REPS=1
N_EVALS=10000


CONTROLLER_ARRAY=()
PROBLEM_TYPE_ARRAY=()
PROBLEM_PATH_ARRAY=()
MAX_SOLVER_TIME_ARRAY=()


i=-1
for INSTANCE_TYPE_TRAIN in "A|B" "A|C" "B|C";do
    for INSTANCE_INDEX_TRAIN in "1" "2" "3" "4" "5"; do
        for INSTANCE_INDEX_TEST in "1" "2" "3" "4" "5"; do
            for INSTANCE_TYPE_TEST in "A" "B" "C"; do

                i=$((i+1))

                if [ "$INSTANCE_INDEX_TRAIN" == "$INSTANCE_INDEX_TEST" ]; then
                    continue
                fi

                list_to_array $INSTANCE_TYPE_TRAIN
                INSTANCE_TYPE_TRAIN_PAIR=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")

                CONTROLLER_NAME_PREFIX="${INSTANCE_TYPE_TRAIN_PAIR[0]}${INSTANCE_TYPE_TRAIN_PAIR[1]}${INSTANCE_INDEX_TRAIN}"
                EXPERIMENT_FOLDER_NAME="src/experiments/permus_multi/results/qap_cut_multi_vs_mono"


                CONTROLLER_ARRAY+=("${EXPERIMENT_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
                PROBLEM_TYPE_ARRAY+=("qap")
                PROBLEM_PATH_ARRAY+=(`ls src/experiments/permus/instances/transfer_qap_cut_instances/${INSTANCE_TYPE_TEST}${INSTANCE_INDEX_TEST}*`)
                MAX_SOLVER_TIME_ARRAY+=("0.25")



            done
        done
    done
done



CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")
MAX_SOLVER_TIME_ARRAY=$(to_list "${MAX_SOLVER_TIME_ARRAY[@]}")



TESTING_JOB_ID=`sbatch --parsable --dependency=afterok:${TRAINING_JOB_ID} --export=CONTROLLER_ARRAY=${CONTROLLER_ARRAY},PROBLEM_TYPE_ARRAY=${PROBLEM_TYPE_ARRAY},PROBLEM_PATH_ARRAY=${PROBLEM_PATH_ARRAY},MAX_SOLVER_TIME_ARRAY=${MAX_SOLVER_TIME_ARRAY},MEASURE_RESPONSES=${MEASURE_RESPONSES},TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS} --array=0-$i src/experiments/permus/scripts/hip_test_array.sl`


if [ -z "$RESPONSE_PATH" ]; then
    RESPONSE_PATH=/dev/null
fi
if [ -z "$SCORE_PATH" ]; then
    SCORE_PATH=/dev/null
fi
cat > script_2828a8741ae82e71b77975df5ec94c25.sh <<EOF
#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j_out.txt
#SBATCH --error=out/slurm_%j_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=2G
#SBATCH --cpus-per-task=2 # number of CPUs
#SBATCH --time=0-00:15:00 #Walltime
#SBATCH -p short

SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB}



if [ "$MEASURE_RESPONSES" == "true" ]; then
    cat ${TMP_RES_PATH}/response_* > ${RESPONSE_PATH}
elif [ "$MEASURE_RESPONSES" == "false" ]; then
    cat ${TMP_RES_PATH}/score_* > ${SCORE_PATH}
else
    echo "MEASURE_RESPONSES = $MEASURE_RESPONSES not set correctly"
    exit 1
fi

rm script_2828a8741ae82e71b77975df5ec94c25.sh
EOF


sbatch --dependency=afterok:$TESTING_JOB_ID script_2828a8741ae82e71b77975df5ec94c25.sh
