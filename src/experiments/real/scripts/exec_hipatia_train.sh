#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=72G
#SBATCH --cpus-per-task=72 # number of CPUs
#SBATCH --time=1-05:30:00 #Walltime
#SBATCH -p large


# # # #!/bin/bash
# # # ###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
# # # #SBATCH --output=out/slurm_%j.txt
# # # #SBATCH --error=err/slurm_err_%j.txt
# # # #SBATCH --ntasks=1 # number of tasks
# # # #SBATCH --ntasks-per-node=1 #number of tasks per node
# # # #SBATCH --mem=12G
# # # #SBATCH --cpus-per-task=12 # number of CPUs
# # # #SBATCH --time=0-00:30:00 #Walltime
# # # #SBATCH -p short




if [[ "$#" -ne 2  ]] ; then
    echo 'Please provide the index of the problem and the dimension. $# parameters where provided. Two are needed. Example: '
    echo ""
    echo 'script.sh 1 200'
    echo ""
    echo 'Exitting...'
    exit 1
fi


SRCDIR=`pwd`



cp ./* -v -r $SCRATCH_JOB
# mkdir $SCRATCH_JOB/data
# cp $dsname -v $SCRATCH_JOB/data
cd $SCRATCH_JOB

# echo `pwd`
# echo `ls`
# echo `ls data`
cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=false
CFLAGS=-fopenmp -std=c++11 -pthread

PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF

make

echo "-compiled-"

cat > tmp.ini <<EOF
; config file for train in hpc hipatia

[Global] 
mode = train
PROBLEM_NAME = real_func

[NEAT]
MAX_TRAIN_TIME = 86400
POPSIZE = 500
THREADS = 72
N_EVALS = 4
N_REEVALS_TOP_5_PERCENT = 1
N_EVALS_TO_UPDATE_BK = 200

SEARCH_TYPE = phased
SEED = 2





[Controller]
POPSIZE = 500
PROBLEM_INDEX = $1
PROBLEM_DIMENSIONS = $2
MAX_EVALS_PSO = 50000


EOF




date
./neat "tmp.ini"
date


cp "controllers_trained_with_F$1" -v -r $SRCDIR



