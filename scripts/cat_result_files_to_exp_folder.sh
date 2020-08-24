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







cat ${TMP_RES_PATH}/scor* > ${SCORE_PATH}
if [ "$COMPUTE_RESPONSE" == "true" ]; then
    cat ${TMP_RES_PATH}/respon* > ${RESPONSE_PATH}
elif [ "$COMPUTE_RESPONSE" == "false" ]; then
    echo "Measure responses not copyed."
else
    echo "COMPUTE_RESPONSE = $COMPUTE_RESPONSE not set correctly"
    exit 1
fi