#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=8G
#SBATCH --cpus-per-task=8 # number of CPUs
#SBATCH --time=0-06:00:00 #Walltime
#SBATCH -p medium
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





if [[ "$#" -ne 3  ]] ; then
    echo 'Please provide the name of the problem, the name of the instance and the path to the controller. $# parameters where provided. 3 are needed. Example: '
    echo ""
    echo 'script.sh qap tai35a.dat.dat experiment_results/inter_instance_transfer/qap_tai35a/experiment_1/fittest_1'
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
mode = test ;


; Configure parameters of NEAT, relevant only during training
[NEAT]
MAX_TRAIN_TIME = 43200 ; time in seconds
POPSIZE = 504 ;
THREADS = 72 ;
N_EVALS = 3 ;
N_REEVALS = 200 ;

SEARCH_TYPE = phased ; only phased, complexify and blended are valid
DELETE_PREVIOUS_EXPERIMENT = true ;
SEED = 2 ;



[Controller]
MAX_TIME_PSO = 0.5 ; Max time the controller has to solve the permutation problem.
POPSIZE = 20 ;
TABU_LENGTH = 40 ;

PROBLEM_TYPE = $1 ; the kind of permutation problem to be solved by the controller.
PROBLEM_PATH = $2 ; 
CONTROLLER_PATH = $3

EOF




date
./neat "tmp.ini"
date

cp experiment_1* -v -r "controllers_$2/"

cp "controllers_$2/" -v -r $SRCDIR


# #end