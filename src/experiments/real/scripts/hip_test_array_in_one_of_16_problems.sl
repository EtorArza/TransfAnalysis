#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=4G
#SBATCH --cpus-per-task=4 # number of CPUs
#SBATCH --output=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_%x_out.txt
#SBATCH --error=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_%x_err.txt
#SBATCH --time=0-06:00:00 #Walltime
#SBATCH -p medium
#SBATCH --exclude=n[001-004]

mkdir ${SCRATCH_JOB}

source scripts/array_to_string_functions.sh 


list_to_array $CONTROLLER_ARRAY
CONTROLLER_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
CONTROLLER=${CONTROLLER_ARRAY[$SLURM_ARRAY_TASK_ID]}


list_to_array $SEED_ARRAY
SEED_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
SEED=${SEED_ARRAY[$SLURM_ARRAY_TASK_ID]}


list_to_array $PROBLEM_INDEX_ARRAY
PROBLEM_INDEX_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_INDEX=${PROBLEM_INDEX_ARRAY[$SLURM_ARRAY_TASK_ID]}
replaced_with=","
PROBLEM_INDEX=${PROBLEM_INDEX//"s_e_p"/$replaced_with}


list_to_array $PROBLEM_DIM_ARRAY
PROBLEM_DIM_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_DIM=${PROBLEM_DIM_ARRAY[$PROBLEM_DIM_ARRAY]}
replaced_with=","
PROBLEM_DIM=${PROBLEM_DIM//"s_e_p"/$replaced_with}







SRCDIR=`pwd`



echo -n "SLURM_ARRAY_TASK_ID: "
echo $SLURM_ARRAY_TASK_ID


cp main.out -v $SCRATCH_JOB 2>&1

cd $SCRATCH_JOB

echo "pwd: `pwd`"





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
PROBLEM_INDEX = ${PROBLEM_INDEX}
PROBLEM_DIM = ${PROBLEM_DIM}
MAX_SOLVER_FE = ${MAX_SOLVER_FE}



FULL_MODEL = ${FULL_MODEL}

EOF


echo "---conf file begin---"

cat tmp.ini

echo "---conf file end---"


date
srun main.out "tmp.ini"
date

rm main.out



# echo "[${CONTROLLER},${PROBLEM_INDEX},${PROBLEM_DIM},${SOLVER_POPSIZE},${MAX_SOLVER_FE}," >> "${TMP_RES_PATH}/score_tmp_${SLURM_JOB_ID}.txt"
# echo "[${CONTROLLER},${PROBLEM_INDEX},${PROBLEM_DIM},${SOLVER_POPSIZE},${MAX_SOLVER_FE}," >> "${TMP_RES_PATH}/responses_tmp_${SLURM_JOB_ID}.txt"

cat "score.txt" >> "${TMP_RES_PATH}/score_tmp_${SLURM_JOB_ID}.txt"
cat "responses.txt" >> "${TMP_RES_PATH}/responses_tmp_${SLURM_JOB_ID}.txt"

cd ..
rm ${SCRATCH_JOB} -r