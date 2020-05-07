#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j_out.txt
#SBATCH --error=out/slurm_%j_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=32G
#SBATCH --cpus-per-task=32 # number of CPUs
#SBATCH --time=2-00:00:00 #Walltime
#SBATCH -p large

mkdir ${SCRATCH_JOB}


SRCDIR=`pwd`
INSTANCE=$PROBLEM_PATH


instance_path=`dirname $INSTANCE`


mkdir -p "$SCRATCH_JOB/$instance_path" && cp $INSTANCE "$SCRATCH_JOB/$instance_path" -v
cp neat -v $SCRATCH_JOB

cd $SCRATCH_JOB


cat > tmp.ini <<EOF
; temporal config file for train in hpc hipatia


[Global]
MODE = train
PROBLEM_NAME = true_ranking_eval


MAX_TRAIN_TIME = $MAX_TRAIN_TIME
POPSIZE = $POPSIZE
THREADS = 32
N_EVALS = $N_EVALS



SEARCH_TYPE = phased
SEED = 2



MAX_SOLVER_TIME = $MAX_SOLVER_TIME
POPSIZE = 20
TABU_LENGTH = 40

PROBLEM_TYPE = $PROBLEM_TYPE
PROBLEM_PATH = $PROBLEM_PATH



EOF




date
srun neat "tmp.ini"
date

cp true_ranking_res.csv $SRCDIR -v
cp samples_to_estimate_prob_of_selecting_best.csv $SRCDIR -v