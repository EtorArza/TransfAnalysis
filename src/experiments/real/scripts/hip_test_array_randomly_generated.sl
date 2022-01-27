#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --output=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_out.txt
#SBATCH --error=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_err.txt
#SBATCH --mem=8G
#SBATCH --cpus-per-task=2 # number of CPUs
#SBATCH --time=5-00:00:00 #Walltime
#SBATCH -p large
#SBATCH --exclude=n[001-004]




SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB}

source scripts/array_to_string_functions.sh 


SRCDIR=`pwd`


list_to_array $CONTROLLER_ARRAY
CONTROLLER_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
CONTROLLER=${CONTROLLER_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $SEED_ARRAY
SEED_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
SEED=${SEED_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $NLO_ARRAY
NLO_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
NLO=${NLO_ARRAY[$SLURM_ARRAY_TASK_ID]}

LOG_FILE="${LOG_DIR}/test_randomly_generated_${SLURM_ARRAY_TASK_ID}.txt"




cp neat -v $SCRATCH_JOB
#cp src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat -v --parents $SCRATCH_JOB/

cd $SCRATCH_JOB

echo "pwd: `pwd`"


mkdir -p src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/
cat > src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat <<EOF
1
1
${DIM}
${SEED}
-1
1
${NLO}
-1
-1
-1
-1
EOF



cat > tmp.ini <<EOF


[Global] 
mode = test
PROBLEM_NAME = real_func

THREADS = ${SLURM_CPUS_PER_TASK}
N_EVALS = ${N_EVALS}
N_REPS = ${N_REPS}
CONTROLLER_PATH = ${CONTROLLER}
PRINT_POSITIONS = false


COMPUTE_RESPONSE = true

SOLVER_POPSIZE = ${SOLVER_POPSIZE}
PROBLEM_INDEX = 0
PROBLEM_DIM = ${DIM}
MAX_SOLVER_FE = ${MAX_SOLVER_FE}

FULL_MODEL = ${FULL_MODEL}

EOF


echo "---conf file begin---"

cat tmp.ini

echo "---conf file end---"


date
srun neat "tmp.ini"
date

rm neat

mkdir $TMP_RES_PATH -p


cat "score.txt" >> "${TMP_RES_PATH}/score_tmp_${SLURM_JOB_ID}.txt"
cat "responses.txt" >> "${TMP_RES_PATH}/responses_tmp_${SLURM_JOB_ID}.txt"

cd ..