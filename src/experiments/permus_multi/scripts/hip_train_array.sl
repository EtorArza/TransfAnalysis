#!/bin/bash


# # ###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
# # #SBATCH --output=out/slurm_%A_%a_out.txt
# # #SBATCH --error=out/slurm_%A_%a_err.txt
# # #SBATCH --ntasks=1 # number of tasks
# # #SBATCH --ntasks-per-node=1 #number of tasks per node
# # #SBATCH --mem=32G
# # #SBATCH --cpus-per-task=32 # number of CPUs
# # #SBATCH --time=0-06:00:00 #Walltime
# # #SBATCH -p medium
# # #SBATCH --exclude=n[001-004,017-018]




###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%A_%a_out.txt
#SBATCH --error=out/slurm_%A_%a_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=4G
#SBATCH --cpus-per-task=4 # number of CPUs
#SBATCH --time=0-0:30:00 #Walltime
#SBATCH -p short
#SBATCH --exclude=n[001-004,017-018]


SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB}



source scripts/array_to_string_functions.sh 

replaced_with=","
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY//"xyz_comma_xyz"/$replaced_with}

list_to_array $PROBLEM_TYPE_ARRAY
PROBLEM_TYPE_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_TYPE=${PROBLEM_TYPE_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $CONTROLLER_NAME_PREFIX_ARRAY
CONTROLLER_NAME_PREFIX_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
CONTROLLER_NAME_PREFIX=${CONTROLLER_NAME_PREFIX_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $EXPERIMENT_FOLDER_NAME_ARRAY
EXPERIMENT_FOLDER_NAME_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
EXPERIMENT_FOLDER_NAME=${EXPERIMENT_FOLDER_NAME_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $SEED_ARRAY
SEED_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
SEED=${SEED_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $POPSIZE_ARRAY
POPSIZE_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
POPSIZE=${POPSIZE_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $MAX_SOLVER_TIME_ARRAY
MAX_SOLVER_TIME_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
MAX_SOLVER_TIME=${MAX_SOLVER_TIME_ARRAY[$SLURM_ARRAY_TASK_ID]}

echo -n "COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY: " 
echo $COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY


list_to_array $COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS=${COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS_ARRAY[$SLURM_ARRAY_TASK_ID]}


SRCDIR=`pwd`

echo -n "COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS: " 
echo $COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS

echo -n "SLURM_ARRAY_TASK_ID: "
echo $SLURM_ARRAY_TASK_ID


list_to_array_with_comma $COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS
ARRAY_OF_INSTANCES=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")


instance_path=`dirname ${ARRAY_OF_INSTANCES[0]}`

echo -n "instance_path: "
echo $instance_path




mkdir -p "$SCRATCH_JOB/$instance_path"

for PROBLEM_PATH in "${ARRAY_OF_INSTANCES[@]}"
do
    cp $PROBLEM_PATH "$SCRATCH_JOB/$instance_path" -v
done 



cp neat -v $SCRATCH_JOB

cd $SCRATCH_JOB

echo "pwd: `pwd`"


cat > tmp.ini <<EOF
; temporal config file for train in hpc hipatia


[Global]
MODE = train
PROBLEM_NAME = permu_multi


MAX_TRAIN_TIME = 40
POPSIZE = $POPSIZE
THREADS = $SLURM_CPUS_PER_TASK

SEARCH_TYPE = phased
SEED = $SEED

CONTROLLER_NAME_PREFIX = $CONTROLLER_NAME_PREFIX
EXPERIMENT_FOLDER_NAME = $EXPERIMENT_FOLDER_NAME

MAX_SOLVER_TIME = $MAX_SOLVER_TIME


PROBLEM_TYPE = $PROBLEM_TYPE
PROBLEM_PATH = dummy_path
COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS = $COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS

EOF

echo "---conf file begin---"

cat tmp.ini

echo "---conf file end---"


date
srun neat "tmp.ini"
date

rm neat