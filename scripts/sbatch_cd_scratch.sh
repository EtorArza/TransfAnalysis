#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%A_%a_out.txt
#SBATCH --error=err/slurm_%A_%a_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=4G
#SBATCH --cpus-per-task=1 # number of CPUs
#SBATCH --time=0-00:30:00 #Walltime
#SBATCH -p short

mkdir ${SCRATCH_JOB}


echo "Scratch_job: ${SCRATCH_JOB}"

#mkdir ${SCRATCH_JOB}

#cd ${SCRATCH_JOB}

#pwd

