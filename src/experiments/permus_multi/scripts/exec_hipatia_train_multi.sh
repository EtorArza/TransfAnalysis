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



## qap n_of_instances path_0 path_1 path_2

# # # #!/bin/bash
# # # ###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
# # #SBATCH --output=out/slurm_%A_%a_out.txt
# # #SBATCH --error=out/slurm_%A_%a_err.txt
# # # #SBATCH --ntasks=1 # number of tasks
# # # #SBATCH --ntasks-per-node=1 #number of tasks per node
# # # #SBATCH --mem=12G
# # # #SBATCH --cpus-per-task=12 # number of CPUs
# # # #SBATCH --time=0-00:30:00 #Walltime
# # # #SBATCH -p short

SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB}




SRCDIR=`pwd`

cd $SCRATCH_JOB
mkdir "src"
cd "src"
mkdir "experiments"
cd $SRCDIR


cp src/experiments -v -r $SCRATCH_JOB/src
cp neat -v $SCRATCH_JOB
cd $SCRATCH_JOB

cat > tmp.ini <<EOF
; config file for train in hpc hipatia


[Global] 
mode = train
PROBLEM_NAME = permu_multi


MAX_TRAIN_TIME = 18000
POPSIZE = 512
THREADS = 32
N_EVALS = 5
N_REEVALS_TOP_5_PERCENT = 40
N_EVALS_TO_UPDATE_BK = 2024



SEARCH_TYPE = phased
SEED = 2



POPSIZE = 20
TABU_LENGTH = 40

PROBLEM_TYPE = $1
N_PROBLEMS = $2

PROBLEM_PATH_0 = $3
MAX_SOLVER_TIME_FOR_EACH_INSTANCE_0 = $4

PROBLEM_PATH_1 = $5
MAX_SOLVER_TIME_FOR_EACH_INSTANCE_1 = $6

PROBLEM_PATH_2 = $7
MAX_SOLVER_TIME_FOR_EACH_INSTANCE_2 = $8


EOF




date
srun neat "tmp.ini"
date

echo "$2"

filename="${instance_path##*/}"
instancename="${filename%%.*}"

echo "$instancename"


cp "controllers_trained_with_${instancename}"* -v -r $SRCDIR/src/experiments/permus_multi/results/controllers/





