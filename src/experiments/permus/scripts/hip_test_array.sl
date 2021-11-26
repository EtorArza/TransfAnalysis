#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_out.txt
#SBATCH --error=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=16G
#SBATCH --cpus-per-task=4 # number of CPUs
#SBATCH --time=0-00:30:00 #Walltime
#SBATCH -p short
#SBATCH --exclude=n[001-004]



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


list_to_array $CONTROLLER_ARRAY
CONTROLLER_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
CONTROLLER=${CONTROLLER_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $PROBLEM_TYPE_ARRAY
PROBLEM_TYPE_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_TYPE=${PROBLEM_TYPE_ARRAY[$SLURM_ARRAY_TASK_ID]}

list_to_array $PROBLEM_PATH_ARRAY
PROBLEM_PATH_ARRAY=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")
PROBLEM_PATH=${PROBLEM_PATH_ARRAY[$SLURM_ARRAY_TASK_ID]}

LOG_FILE="${LOG_DIR}/test_controller_$(basename $CONTROLLER)_${SEED}_problem_$(basename $PROBLEM_PATH)_${SLURM_ARRAY_TASK_ID}.txt"







echo -n "CONTROLLER: " >> ${LOG_FILE} && echo $CONTROLLER >> ${LOG_FILE}
echo -n "PROBLEM_TYPE: " >> ${LOG_FILE} && echo $PROBLEM_TYPE >> ${LOG_FILE}
echo -n "PROBLEM_PATH: " >> ${LOG_FILE} && echo $PROBLEM_PATH >> ${LOG_FILE}
echo -n "TMP_RES_PATH: " >> ${LOG_FILE} && echo $TMP_RES_PATH >> ${LOG_FILE}

SRCDIR=`pwd`


mkdir $TMP_RES_PATH -p
cp -v --parents $PROBLEM_PATH $SCRATCH_JOB >> ${LOG_FILE}
cp -v --parents $CONTROLLER $SCRATCH_JOB  >> ${LOG_FILE}
cp neat -v $SCRATCH_JOB  >> ${LOG_FILE}



cd $SCRATCH_JOB
cat > tmp.ini <<EOF
; config file for test in hpc hipatia


[Global] 
MODE = test ;
PROBLEM_NAME = permu


THREADS = $SLURM_CPUS_PER_TASK ;
CONTROLLER_PATH = ${CONTROLLER} ; 
COMPUTE_RESPONSE = $COMPUTE_RESPONSE
N_REPS = $N_REPS
N_EVALS = $N_EVALS


MAX_SOLVER_FE = $MAX_SOLVER_FE ; 

PROBLEM_TYPE = $PROBLEM_TYPE ; 
PROBLEM_PATH = $PROBLEM_PATH ; 

EOF
echo "---conf file begin---" >> ${LOG_FILE}

cat tmp.ini >> ${LOG_FILE}

echo "---conf file end---" >> ${LOG_FILE}

date >> ${LOG_FILE}
srun neat "tmp.ini" >> ${LOG_FILE} 2>&1
date >> ${LOG_FILE}

rm neat

mkdir $TMP_RES_PATH -p


cat "score.txt" >> "${TMP_RES_PATH}/score_tmp_${SLURM_JOB_ID}.txt"
cat "responses.txt" >> "${TMP_RES_PATH}/responses_tmp_${SLURM_JOB_ID}.txt"

cd ..
