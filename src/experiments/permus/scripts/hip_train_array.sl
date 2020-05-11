#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%A_%a_out.txt
#SBATCH --error=out/slurm_%A_%a_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=32G
#SBATCH --cpus-per-task=32 # number of CPUs
#SBATCH --time=0-06:00:00 #Walltime
#SBATCH -p medium
#SBATCH --exclude=n[001-004,017-018]




# # #!/bin/bash
# # ###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
# # #SBATCH --output=out/slurm_%A_%a_out.txt
# # #SBATCH --error=out/slurm_%A_%a_err.txt
# # #SBATCH --ntasks=1 # number of tasks
# # #SBATCH --ntasks-per-node=1 #number of tasks per node
# # #SBATCH --mem=32G
# # #SBATCH --cpus-per-task=32 # number of CPUs
# # #SBATCH --time=0-6:00:00 #Walltime
# # #SBATCH -p medium
# # #SBATCH --exclude=n[001-004,017-018]

SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB}
module load GCC/8.3.0


source scripts/array_to_string_functions.sh 



list_to_array $PROBLEM_TYPE_ARRAY
PROBLEM_TYPE_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_TYPE=${PROBLEM_TYPE_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $PROBLEM_PATH_ARRAY
PROBLEM_PATH_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_PATH=${PROBLEM_PATH_ARRAY[$SLURM_ARRAY_TASK_ID]}

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


SRCDIR=`pwd`

echo -n "PROBLEM_PATH: " 
echo $PROBLEM_PATH

echo -n "SLURM_ARRAY_TASK_ID: "
echo $SLURM_ARRAY_TASK_ID

instance_path=`dirname $PROBLEM_PATH`

echo -n "instance_path: "
echo $instance_path




mkdir -p "$SCRATCH_JOB/$instance_path" && cp $PROBLEM_PATH "$SCRATCH_JOB/$instance_path" -v
cp neat -v $SCRATCH_JOB

cd $SCRATCH_JOB

echo "pwd: `pwd`"


cat > tmp.ini <<EOF
; temporal config file for train in hpc hipatia


[Global]
MODE = train
PROBLEM_NAME = permu


MAX_TRAIN_TIME = 19800
POPSIZE = $POPSIZE
THREADS = $SLURM_CPUS_PER_TASK

SEARCH_TYPE = phased
SEED = $SEED

CONTROLLER_NAME_PREFIX = $CONTROLLER_NAME_PREFIX
EXPERIMENT_FOLDER_NAME = $EXPERIMENT_FOLDER_NAME

MAX_SOLVER_TIME = $MAX_SOLVER_TIME


PROBLEM_TYPE = $PROBLEM_TYPE
PROBLEM_PATH = $PROBLEM_PATH

EOF




date
srun neat "tmp.ini"
date

rm neat