#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_out.txt
#SBATCH --error=/workspace/scratch/jobs/earza/slurm_logs/slurm_%A_%a_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=32G
#SBATCH --cpus-per-task=32 # number of CPUs
#SBATCH --time=5-00:00:00 #Walltime
#SBATCH -p large
#SBATCH --exclude=n[001-004,017-018]
#SBATCH --exclusive




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



source scripts/array_to_string_functions.sh 


PROBLEM_DIM=20




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
${PROBLEM_DIM}
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
THREADS =  $SLURM_CPUS_PER_TASK
EXPERIMENT_FOLDER_NAME = ${EXPERIMENT_FOLDER_NAME}
CONTROLLER_NAME_PREFIX = ${CONTROLLER_NAME_PREFIX}



SEARCH_TYPE = phased
SEED = 2
FULL_MODEL = false



MAX_SOLVER_FE = ${MAX_SOLVER_FE}
SOLVER_POPSIZE = ${SOLVER_POPSIZE}



COMMA_SEPARATED_PROBLEM_INDEX_LIST = 0
COMMA_SEPARATED_PROBLEM_DIM_LIST   = ${PROBLEM_DIM}


EOF


echo "---conf file begin---"

cat tmp.ini

echo "---conf file end---"


date
srun neat "tmp.ini"
date

rm neat