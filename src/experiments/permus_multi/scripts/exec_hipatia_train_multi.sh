#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=32G
#SBATCH --cpus-per-task=32 # number of CPUs
#SBATCH --time=2-01:30:00 #Walltime
#SBATCH -p large
#SBATCH --exclude=n[001-004,017-018]



## qap n_of_instances path_0 path_1 path_2

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
PROBLEM_NAME = permu_multi


[NEAT]
MAX_TRAIN_TIME = 172800
POPSIZE = 640
THREADS = 32
N_EVALS = 5
N_REEVALS_TOP_5_PERCENT = 20
N_EVALS_TO_UPDATE_BK = 1280



SEARCH_TYPE = phased
SEED = 2



[Controller]
POPSIZE = 20
TABU_LENGTH = 40

PROBLEM_TYPE = $1
N_PROBLEMS = $2

PROBLEM_PATH_0 = $3
MAX_TIME_PSO_FOR_EACH_INSTANCE_0 = $4

PROBLEM_PATH_1 = $5
MAX_TIME_PSO_FOR_EACH_INSTANCE_1 = $6

PROBLEM_PATH_2 = $7
MAX_TIME_PSO_FOR_EACH_INSTANCE_2 = $8


EOF




date
./neat "tmp.ini"
date

instance_path=$3


echo "$2"

filename="${instance_path##*/}"
instancename="${filename%%.*}"



cp "controllers_trained_with_${instancename}"* -v -r $SRCDIR/src/experiments/permus_multi/results/controllers/



