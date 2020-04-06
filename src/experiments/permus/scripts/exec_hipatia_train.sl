#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=32G
#SBATCH --cpus-per-task=32 # number of CPUs
#SBATCH --time=0-13:00:00 #Walltime
#SBATCH -p large
#SBATCH --exclude=n[001-004,017-018]


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




if [[ "$#" -ne 3  ]] ; then
    echo 'Please provide the name of the problem, the path of the instance and and max_pso_time. $# parameters where provided. 3 are needed. Example: '
    echo ""
    echo 'script.sh qap tai35a.dat.dat 0.5'
    echo ""
    echo 'Exitting...'
    exit 1
fi


SRCDIR=`pwd`

cd $SCRATCH_JOB
mkdir "src"
cd "src"
mkdir "experiments"
cd $SRCDIR


cp src/experiments -v -r $SCRATCH_JOB/src
cp neat -v $SCRATCH_JOB
cd $SCRATCH_JOB


N_EVALS=20
cat > tmp.ini <<EOF
; temporal config file for train in hpc hipatia


[Global]
MODE = train
PROBLEM_NAME = permu


[NEAT]
MAX_TRAIN_TIME = 43200
POPSIZE = 640
THREADS = 32
N_EVALS = $N_EVALS



SEARCH_TYPE = phased
SEED = 2



[Controller]
MAX_TIME_PSO = $3


PROBLEM_TYPE = $1
PROBLEM_PATH = $2

EOF




date
./neat "tmp.ini"
date

instance_path=$2


echo "$2"

filename="${instance_path##*/}"
instancename="${filename%%.*}"

echo "$instancename"


DESTINATION_FOLDER="$SRCDIR/src/experiments/permus/results/controllers/"


mkdir -p ${DESTINATION_FOLDER}
cp "controllers_trained_with_$instancename"* -v -r "${DESTINATION_FOLDER}"

exit 0
