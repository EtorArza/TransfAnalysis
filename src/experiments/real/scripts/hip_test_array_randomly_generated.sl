#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --output=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_out.txt
#SBATCH --error=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_err.txt
#SBATCH --mem=8G
#SBATCH --cpus-per-task=4 # number of CPUs
#SBATCH --time=5-00:00:00 #Walltime
#SBATCH -p large
#SBATCH --exclude=n[001-004]




SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB}

SRCDIR=`pwd`


list_to_array $NEAT_POPSIZE_ARRAY
NEAT_POPSIZE_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
NEAT_POPSIZE=${NEAT_POPSIZE_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $CONTROLLER_NAME_PREFIX_ARRAY
CONTROLLER_NAME_PREFIX_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
CONTROLLER_NAME_PREFIX=${CONTROLLER_NAME_PREFIX_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $EXPERIMENT_FOLDER_NAME_ARRAY
EXPERIMENT_FOLDER_NAME_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
EXPERIMENT_FOLDER_NAME=${EXPERIMENT_FOLDER_NAME_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $SEED_ARRAY
SEED_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
SEED=${SEED_ARRAY[$SLURM_ARRAY_TASK_ID]}



list_to_array $PROBLEM_NLO_ARRAY
PROBLEM_NLO_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_NLO=${PROBLEM_NLO_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $PROBLEM_DIM_ARRAY
PROBLEM_DIM_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_DIM=${PROBLEM_DIM_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $SOLVER_POPSIZE_ARRAY
SOLVER_POPSIZE_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
SOLVER_POPSIZE=${SOLVER_POPSIZE_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $MAX_SOLVER_FE_ARRAY
MAX_SOLVER_FE_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
MAX_SOLVER_FE=${MAX_SOLVER_FE_ARRAY[$SLURM_ARRAY_TASK_ID]}






LOG_FILE="${LOG_DIR}/test_randomly_generated_${SLURM_ARRAY_TASK_ID}.txt"




echo -n "SLURM_ARRAY_TASK_ID: " >> ${LOG_FILE}
echo $SLURM_ARRAY_TASK_ID >> ${LOG_FILE}


cp neat -v $SCRATCH_JOB >> ${LOG_FILE}
#cp src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat -v --parents $SCRATCH_JOB/

cd $SCRATCH_JOB

echo "pwd: `pwd`" >> ${LOG_FILE}



mkdir -p src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/
cat > src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat <<EOF
1
1
4
1
2
1
${PROBLEM_NLO}
0.02
-1
-1
-1
EOF



cat > tmp.ini <<EOF


[Global] 
mode = train
PROBLEM_NAME = real_func

POPSIZE = ${NEAT_POPSIZE}
MAX_TRAIN_ITERATIONS = ${MAX_TRAIN_ITERATIONS}
MAX_TRAIN_TIME = ${MAX_TRAIN_TIME}
THREADS = ${SLURM_CPUS_PER_TASK}
EXPERIMENT_FOLDER_NAME = ${EXPERIMENT_CONTROLLER_FOLDER_NAME}
CONTROLLER_NAME_PREFIX = ${CONTROLLER_NAME_PREFIX}
FULL_MODEL = ${FULL_MODEL}


SEARCH_TYPE = phased
SEED = ${SEED}



MAX_SOLVER_FE = ${MAX_SOLVER_FE}
SOLVER_POPSIZE = ${SOLVER_POPSIZE}



COMMA_SEPARATED_PROBLEM_INDEX_LIST = 0
COMMA_SEPARATED_PROBLEM_DIM_LIST = ${PROBLEM_DIM}

EOF


echo "---conf file begin---"

cat tmp.ini

echo "---conf file end---"


date
srun neat "tmp.ini"
date

rm neat

mkdir $TMP_RES_PATH -p


echo "[${EXP_NAME},${PROBLEM_NLO},${PROBLEM_DIM},${SOLVER_POPSIZE},${MAX_SOLVER_FE}]" >> "${TMP_RES_PATH}/score_journal_out_${SLURM_JOB_ID}.txt"
echo "[${EXP_NAME},${PROBLEM_NLO},${PROBLEM_DIM},${SOLVER_POPSIZE},${MAX_SOLVER_FE}]" >> "${TMP_RES_PATH}/responses_journal_${SLURM_JOB_ID}.txt"

cat "score.txt" >> "${TMP_RES_PATH}/score_journal_out_${SLURM_JOB_ID}.txt"
cat "responses.txt" >> "${TMP_RES_PATH}/responses_journal_${SLURM_JOB_ID}.txt"

cd ..

date