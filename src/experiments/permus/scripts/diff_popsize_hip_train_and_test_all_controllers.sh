#!/bin/bash

source scripts/array_to_string_functions.sh


srun bash scripts/make_hip.sh

SRCDIR=`pwd`
EXPERIMENT_FOLDER_NAME="${SRCDIR}/src/experiments/permus/results/popsize_exp"

TRAINING_JOB_IDS=""
for SEED in "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" "14" "15" "16" "17" "18" "19" "20" "21";do
    for POPSIZE in "128" "256" "512" "1024" "2048"; do
        INSTANCES="src/experiments/permus/instances/qap/popsize_instance/*.qap"
        PROBLEM_TYPE="qap"
        NUM_INSTANCES=$( ls -1 $INSTANCES | wc -l)
        NUM_INSTANCES=$(($NUM_INSTANCES - 1))
        CONTROLLER_NAME_PREFIX="popsize${POPSIZE}_SEED${SEED}"
        TRAINING_JOB_IDS=`sbatch --parsable --export=SEED=$SEED,PROBLEM_TYPE=$PROBLEM_TYPE,POPSIZE=$POPSIZE,INSTANCES=$INSTANCES,MAX_TIME_PSO=0.25,EXPERIMENT_FOLDER_NAME=${EXPERIMENT_FOLDER_NAME},CONTROLLER_NAME_PREFIX=${CONTROLLER_NAME_PREFIX} --array=0-$NUM_INSTANCES src/experiments/permus/scripts/hip_train_array.sl`:$TRAINING_JOB_IDS
    done
done
TRAINING_JOB_IDS=${TRAINING_JOB_IDS%?}


MEASURE_RESPONSES="false"
SCORE_PATH="src/experiments/permus/results/popsize_exp/result_popsize_experiment.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname $SCORE_PATH)


TESTING_JOB_IDS=""
for SEED in "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" "14" "15" "16" "17" "18" "19" "20" "21";do
    for POPSIZE in "128" "256" "512" "1024" "2048"; do
        NUM_JOBS=0
        CONTROLLER_NAME_PREFIX="popsize${POPSIZE}_SEED${SEED}"
        CONTROLLERS="src/experiments/permus/results/popsize_exp/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller"
        INSTANCES="src/experiments/permus/instances/qap/popsize_instance/tai75e01.qap"
        N_REPS=1
        N_EVALS=40000
        PROBLEMS="qap"
        TESTING_JOB_IDS=`sbatch --parsable --dependency=afterok:${TRAINING_JOB_IDS} --export=CONTROLLERS=${CONTROLLERS},PROBLEMS=${PROBLEMS},INSTANCES=${INSTANCES},MAX_TIME_PSO=0.25,${MEASURE_RESPONSES}=false,TMP_RES_PATH=${TMP_RES_PATH},N_REPS=${N_REPS},N_EVALS=${N_EVALS} --array=0-$NUM_JOBS src/experiments/permus/scripts/hip_test_array.sl`:$TESTING_JOB_IDS
    done
done
TESTING_JOB_IDS=${TESTING_JOB_IDS%?}


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


sbatch --dependency=afterok:$TESTING_JOB_IDS script_2828a8741ae82e71b77975df5ec94c25.sh
