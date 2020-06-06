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


if [ -z "$RESPONSE_PATH" ]; then
    RESPONSE_PATH=/dev/null
fi
if [ -z "$SCORE_PATH" ]; then
    SCORE_PATH=/dev/null
fi







cat ${TMP_RES_PATH}/score_* > ${SCORE_PATH}
if [ "$MEASURE_RESPONSES" == "true" ]; then
    cat ${TMP_RES_PATH}/response_* > ${RESPONSE_PATH}
elif [ "$MEASURE_RESPONSES" == "false" ]; then
    echo "Measure responses not copyed."
else
    echo "MEASURE_RESPONSES = $MEASURE_RESPONSES not set correctly"
    exit 1
fi