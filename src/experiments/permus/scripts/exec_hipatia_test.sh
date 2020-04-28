#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=3G
#SBATCH --cpus-per-task=3 # number of CPUs
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





SRCDIR=`pwd`


TMP_PATH=${SRCDIR}/"tmp"/$(dirname $5)

mkdir -p ${TMP_PATH}
cd $SCRATCH_JOB
mkdir "src"
cd "src"
mkdir "experiments"
cd $SRCDIR

sleep "$((${RANDOM} % 5)).$((${RANDOM} % 999))"

cp src/experiments -v -r $SCRATCH_JOB/src
cp neat -v $SCRATCH_JOB
cd $SCRATCH_JOB



cat > tmp.ini <<EOF
; config file for test in hpc hipatia


[Global] 
MODE = test ;
PROBLEM_NAME = permu


THREADS = 32 ;
N_EVALS = 5 ;
N_REPS = 20 ;
CONTROLLER_PATH = $3 ; 
COMPUTE_RESPONSE = $6

MAX_SOLVER_TIME = $4 ; 
POPSIZE = 20 ;
TABU_LENGTH = 40 ;
N_EVALS = 500 ;
N_REPS = 20 ;

PROBLEM_TYPE = $1 ; 
PROBLEM_PATH = $2 ; 

EOF


date
./neat "tmp.ini"

sleep "$((${RANDOM} % 5)).$((${RANDOM} % 999))"


#cat "result.txt" >> "$SRCDIR/$5"
cat "result.txt" >> "${TMP_PATH}/score_journal_out_${SLURM_JOB_ID}.txt"

#cat "responses.txt" >> "$SRCDIR/src/experiments/permus/results/analyze_outputs/responses_journal.txt"
cat "responses.txt" >> "${TMP_PATH}/responses_journal_${SLURM_JOB_ID}.txt"

cd ..
rm ${SLURM_JOB_ID} -r

date


# #end
