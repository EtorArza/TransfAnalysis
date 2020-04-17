#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=32G
#SBATCH --cpus-per-task=32 # number of CPUs
#SBATCH --time=0-00:30:00 #Walltime
#SBATCH -p short
#SBATCH --exclude=n[001-004,017-018]



# # # #!/bin/bash
# # # ###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
# # # #SBATCH --output=out/slurm_%j.txt
# # # #SBATCH --error=err/slurm_err_%j.txt
# # # #SBATCH --ntasks=1 # number of tasks
# # # #SBATCH --ntasks-per-node=1 #number of tasks per node
# # # #SBATCH --mem=2G
# # # #SBATCH --cpus-per-task=2 # number of CPUs
# # # #SBATCH --time=0-00:30:00 #Walltime
# # # #SBATCH -p short

source scripts/array_to_string_functions.sh 


list_to_array $CONTROLLERS
CONTROLLERS=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")

list_to_array $PROBLEMS
PROBLEMS=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")

list_to_array $INSTANCES
INSTANCES=("${BITRISE_CLI_LAST_PARSED_LIST[@]}")




CONTROLLER=${CONTROLLERS[$SLURM_ARRAY_TASK_ID]}
PROBLEM=${PROBLEMS[$SLURM_ARRAY_TASK_ID]}
INSTANCE=${INSTANCES[$SLURM_ARRAY_TASK_ID]}

echo -n "CONTROLLER: " && echo $CONTROLLER
echo -n "PROBLEM: " && echo $PROBLEM
echo -n "INSTANCE: " && echo $INSTANCE

SRCDIR=`pwd`


mkdir $TMP_RES_PATH -p
cp -v --parents $INSTANCE $SCRATCH_JOB
cp -v --parents $CONTROLLER $SCRATCH_JOB 
cp neat -v $SCRATCH_JOB 



cd $SCRATCH_JOB
cat > tmp.ini <<EOF
; config file for test in hpc hipatia


[Global] 
MODE = test ;
PROBLEM_NAME = permu


THREADS = 32 ;
CONTROLLER_PATH = $CONTROLLER ; 
COMPUTE_RESPONSE = $MEASURE_RESPONSES


MAX_TIME_PSO = $MAX_TIME_PSO ; 
POPSIZE = $POPSIZE ;

PROBLEM_TYPE = $PROBLEM ; 
PROBLEM_PATH = $INSTANCE ; 

EOF


date
./neat "tmp.ini"
date


#cat "result.txt" >> "$SRCDIR/$5"
cat "result.txt" >> "${TMP_RES_PATH}/score_journal_out_${SLURM_JOB_ID}.txt"

#cat "responses.txt" >> "$SRCDIR/src/experiments/permus/results/analyze_outputs/responses_journal.txt"
cat "responses.txt" >> "${TMP_RES_PATH}/responses_journal_${SLURM_JOB_ID}.txt"

cd ..
rm ${SLURM_JOB_ID} -r

date


# #end
