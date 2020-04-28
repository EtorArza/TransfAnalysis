#!/bin/bash

source scripts/array_to_string_functions.sh


COMPILE_JOB_ID=`sbatch --parsable scripts/make_hip.sh`

SRCDIR=`pwd`

PROBLEM_TYPE_ARRAY=()
PROBLEM_PATH_ARRAY=()
POPSIZE_ARRAY=()
CONTROLLER_NAME_PREFIX_ARRAY=()
EXPERIMENT_FOLDER_NAME_ARRAY=()
CONTROLLER_ARRAY=()
SEED_ARRAY=()
MAX_SOLVER_TIME_ARRAY=()

i=-1
for SEED in "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" "14" "15" "16" "17" "18" "19" "20" "21";do
    for POPSIZE in "128" "256" "512" "1024" "2048"; do
        i=$((i+1))

        EXPERIMENT_FOLDER_NAME="src/experiments/permus/results/popsize_exp"
        CONTROLLER_NAME_PREFIX="popsize${POPSIZE}_SEED${SEED}"

        POPSIZE_ARRAY+=(${POPSIZE})
        SEED_ARRAY+=("${SEED}")

        PROBLEM_TYPE_ARRAY+=("qap")
        PROBLEM_PATH_ARRAY+=("src/experiments/permus/instances/diff_popsize_experiment/tai75e01.qap")
        CONTROLLER_NAME_PREFIX_ARRAY+=("popsize${POPSIZE}_SEED${SEED}")
        EXPERIMENT_FOLDER_NAME_ARRAY+=("${SRCDIR}/${EXPERIMENT_FOLDER_NAME}")
        CONTROLLER_ARRAY+=("${EXPERIMENT_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller")
        MAX_SOLVER_TIME_ARRAY+=("0.25")
    done
done

PROBLEM_TYPE_ARRAY=$(to_list "${PROBLEM_TYPE_ARRAY[@]}")
PROBLEM_PATH_ARRAY=$(to_list "${PROBLEM_PATH_ARRAY[@]}")
POPSIZE_ARRAY=$(to_list "${POPSIZE_ARRAY[@]}")
CONTROLLER_NAME_PREFIX_ARRAY=$(to_list "${CONTROLLER_NAME_PREFIX_ARRAY[@]}")
EXPERIMENT_FOLDER_NAME_ARRAY=$(to_list "${EXPERIMENT_FOLDER_NAME_ARRAY[@]}")
CONTROLLER_ARRAY=$(to_list "${CONTROLLER_ARRAY[@]}")
SEED_ARRAY=$(to_list "${SEED_ARRAY[@]}")
MAX_SOLVER_TIME_ARRAY=$(to_list "${MAX_SOLVER_TIME_ARRAY[@]}")


TRAINING_JOB_ID=`sbatch --parsable --dependency=afterok:${COMPILE_JOB_ID} --export=PROBLEM_TYPE_ARRAY=$PROBLEM_TYPE_ARRAY,PROBLEM_PATH_ARRAY=$PROBLEM_PATH_ARRAY,CONTROLLER_NAME_PREFIX_ARRAY=$CONTROLLER_NAME_PREFIX_ARRAY,EXPERIMENT_FOLDER_NAME_ARRAY=$EXPERIMENT_FOLDER_NAME_ARRAY,SEED_ARRAY=$SEED_ARRAY,POPSIZE_ARRAY=$POPSIZE_ARRAY,MAX_SOLVER_TIME_ARRAY=$MAX_SOLVER_TIME_ARRAY --array=0-$i src/experiments/permus/scripts/hip_train_array.sl`





MEASURE_RESPONSES="false"
SCORE_PATH="src/experiments/permus/results/popsize_exp/result_popsize_experiment.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname ${SCORE_PATH})
N_REPS=1
N_EVALS=40000



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
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=2G
#SBATCH --cpus-per-task=2 # number of CPUs
#SBATCH --time=0-00:15:00 #Walltime
#SBATCH -p short

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
