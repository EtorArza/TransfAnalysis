#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=128G
#SBATCH --cpus-per-task=72 # number of CPUs
#SBATCH --time=5-00:00:00 #Walltime
#SBATCH -p large
#SBATCH --exclude=n[001-016]



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





if [[ "$#" -ne 2  ]] ; then
    echo 'Please provide the name of the problem and the name of the instance. $# parameters where provided. Two are needed. Example: '
    echo ""
    echo 'script.sh qap tai35a.dat.dat'
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
mode = train ;  decide wether to train a controller or test it


; Configure parameters of NEAT, relevant only during training
[NEAT]
MAX_TRAIN_TIME = 43200 ; time in seconds
POPSIZE = 504 
THREADS = 72
N_EVALS = 10
N_REEVALS_TOP_5_PERCENT = 200
N_EVALS_TO_UPDATE_BK = 5040



SEARCH_TYPE = phased ;
DELETE_PREVIOUS_EXPERIMENT = true ;
SEED = 2 ;
START_WITHOUT_HIDDEN = true ;



[Controller]
MAX_TIME_PSO = 0.5 ; Max time the controller has to solve the permutation problem.
POPSIZE = 20 ;
TABU_LENGTH = 40 ;

PROBLEM_TYPE = $1 ; the kind of permutation problem to be solved by the controller.
PROBLEM_PATH = $2 ; 

EOF




date
./neat "tmp.ini"
date


cp experiment_1* -v -r $SRCDIR




# #end