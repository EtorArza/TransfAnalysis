#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%A_%a_%x_out.txt
#SBATCH --error=out/slurm_%A_%a_%x_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=4G
#SBATCH --cpus-per-task=1 # number of CPUs
#SBATCH --time=0-00:30:00 #Walltime
#SBATCH -p short

SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB} -p



echo "Scratch_job: ${SCRATCH_JOB}"


cd ${SCRATCH_JOB}

pwd

