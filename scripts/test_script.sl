#!/bin/bash


###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_out.txt
#SBATCH --error=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=4G
#SBATCH --cpus-per-task=1 # number of CPUs
#SBATCH --time=0-0:30:00 #Walltime
#SBATCH -p short

source scripts/array_to_string_functions.sh 


replaced_with=","
array_var=${array_var//"xyz_comma_xyz"/$replaced_with}


echo "$SLURM_ARRAY_TASK_ID"

list_to_array_with_comma ${array_var}
array_var_array=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
res=${array_var_array[$SLURM_ARRAY_TASK_ID]}

echo "${res}"

list_to_array ${array_var}
array_var_array=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
res=${array_var_array[$SLURM_ARRAY_TASK_ID]}

echo "${res}"