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
# # # #SBATCH --mem=4G
# # # #SBATCH --cpus-per-task=4 # number of CPUs
# # # #SBATCH --time=0-00:30:00 #Walltime
# # # #SBATCH -p short








SRCDIR=`pwd`
INSTANCES=($(ls -1 $INSTANCES))
INSTANCE=${INSTANCES[$SLURM_ARRAY_TASK_ID]}


instance_path=dirname $INSTANCE

mkdir -p "$SCRATCH_JOB/$instance_path" && cp $INSTANCE "$SCRATCH_JOB/$instance_path" -v
cp neat -v $SCRATCH_JOB

cd $SCRATCH_JOB




cat > tmp.ini <<EOF
; temporal config file for train in hpc hipatia


[Global]
MODE = train
PROBLEM_NAME = permu


[NEAT]
MAX_TRAIN_TIME = 43200 ; 300
POPSIZE = 640
THREADS = 32
N_EVALS = $N_EVALS
N_REEVALS_TOP_5_PERCENT = 100
UPDATE_BK_EVERY_K_ITERATIONS = 40
SAMPLE_SIZE_UPDATE_BK = $SAMPLE_SIZE_UPDATE_BK
N_SAMPLES_UPDATE_BK = 64


SEARCH_TYPE = phased
SEED = 2



[Controller]
MAX_TIME_PSO = $MAX_TIME_PSO
POPSIZE = 20
TABU_LENGTH = 40

PROBLEM_TYPE = $PROBLEM_TYPE
PROBLEM_PATH = $INSTANCE

EOF




date
./neat "tmp.ini"
date

instance_path=$INSTANCE




filename="${instance_path##*/}"
instancename="${filename%%.*}"

echo "$instancename"


DESTINATION_FOLDER="$SRCDIR/$DESTINATION_FOLDER"


mkdir -p ${DESTINATION_FOLDER}
cp "controllers_trained_with_$instancename"* -v -r "${DESTINATION_FOLDER}"

exit 0
