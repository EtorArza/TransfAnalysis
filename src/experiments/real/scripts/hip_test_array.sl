#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%A_%a_out.txt
#SBATCH --error=out/slurm_%A_%a_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=32G
#SBATCH --cpus-per-task=32 # number of CPUs
#SBATCH --time=0-01:30:00 #Walltime
#SBATCH -p large
#SBATCH --exclude=n[001-004,017-018]
#SBATCH --exclusive



# # # #!/bin/bash
# # # ###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
# # # #SBATCH --output=out/slurm_%j.txt
# # # #SBATCH --error=out/slurm_err_%j.txt
# # # #SBATCH --ntasks=1 # number of tasks
# # # #SBATCH --ntasks-per-node=1 #number of tasks per node
# # # #SBATCH --mem=2G
# # # #SBATCH --cpus-per-task=2 # number of CPUs
# # # #SBATCH --time=0-00:30:00 #Walltime
# # # #SBATCH -p short

SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB}

source scripts/array_to_string_functions.sh 


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





SRCDIR=`pwd`
EXPERIMENT_FOLDER_NAME="${SRCDIR}/src/experiments/real/results/transferability"



echo -n "SLURM_ARRAY_TASK_ID: "
echo $SLURM_ARRAY_TASK_ID


cp neat -v $SCRATCH_JOB
#cp src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat -v --parents $SCRATCH_JOB/

cd $SCRATCH_JOB

echo "pwd: `pwd`"

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
mode = test
PROBLEM_NAME = real_func

POPSIZE = ${NEAT_POPSIZE}
THREADS =  $SLURM_CPUS_PER_TASK
EXPERIMENT_FOLDER_NAME = ${EXPERIMENT_FOLDER_NAME}
CONTROLLER_NAME_PREFIX = ${CONTROLLER_NAME_PREFIX}

COMPUTE_RESPONSE = ${COMPUTE_RESPONSE}

N_EVALS = ${N_EVALS}
N_REPS = ${N_REPS}
CONTROLLER_PATH = ${EXPERIMENT_FOLDER_NAME}/top_controllers/${CONTROLLER_NAME_PREFIX}_best.controller
PRINT_POSITIONS = false

SEED = 2

MAX_SOLVER_FE = ${MAX_SOLVER_FE}
SOLVER_POPSIZE = ${SOLVER_POPSIZE}





PROBLEM_INDEX = 10
PROBLEM_DIM = ${PROBLEM_DIM}

X_LOWER_LIM = 0
X_UPPER_LIM = 1

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

cat "result.txt" >> "${TMP_RES_PATH}/score_journal_out_${SLURM_JOB_ID}.txt"
cat "responses.txt" >> "${TMP_RES_PATH}/responses_journal_${SLURM_JOB_ID}.txt"

cd ..

date