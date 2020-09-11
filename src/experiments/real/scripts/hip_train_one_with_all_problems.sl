#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%A_%a_out.txt
#SBATCH --error=out/slurm_%A_%a_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=72G
#SBATCH --cpus-per-task=72 # number of CPUs
#SBATCH --time=4-00:00:00 #Walltime
#SBATCH -p large
#SBATCH --exclude=n[001-016]
#SBATCH --exclusive


SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB}


SRCDIR=`pwd`
EXPERIMENT_FOLDER_NAME="${SRCDIR}/src/experiments/real/results/comparison_other_pso/controllers_trained_on_all_problems"
CONTROLLER_NAME_PREFIX="trained_with_all_problems"


echo -n "SLURM_ARRAY_TASK_ID: "
echo $SLURM_ARRAY_TASK_ID


cp neat -v $SCRATCH_JOB
#cp src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat -v --parents $SCRATCH_JOB/

cd $SCRATCH_JOB

echo "pwd: `pwd`"




cat > tmp.ini <<EOF

[Global] 
mode = train
PROBLEM_NAME = real_func

POPSIZE = 512
MAX_TRAIN_ITERATIONS = 2000
THREADS = 72
EXPERIMENT_FOLDER_NAME = ${EXPERIMENT_FOLDER_NAME}
CONTROLLER_NAME_PREFIX = ${CONTROLLER_NAME_PREFIX}



SEARCH_TYPE = phased
SEED = 2



MAX_SOLVER_FE = 100000
SOLVER_POPSIZE = 20



COMMA_SEPARATED_PROBLEM_INDEX_LIST = 1,2,3,4,5,7,8,9,11,1,2,3,4,5,7,8,9,11
COMMA_SEPARATED_PROBLEM_DIM_LIST   = 4,4,4,4,4,4,4,4,4,20,20,20,20,20,20,20,20,20


COMMA_SEPARATED_X_LOWER_LIST = -10,-10,-5,-5.12,-15,-600,-5,-15,-100,-2.048,-10,-10,-5,-5.12,-15,-600,-5,-15,-100,-2.048
COMMA_SEPARATED_X_UPPER_LIST = 5,10,10,5,30,600,10,30,100,2.048,5,10,10,5,30,600,10,30,100,2.048
EOF


echo "---conf file begin---"

cat tmp.ini

echo "---conf file end---"


date
srun neat "tmp.ini"
date

rm neat