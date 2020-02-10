#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=32G
#SBATCH --cpus-per-task=32 # number of CPUs
#SBATCH --time=0-00:30:00 #Walltime
#SBATCH -p short
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





if [[ "$#" -ne 6  ]] ; then
    echo 'Please provide the name of the problem, the name of the instance and the path to the controller, max_pso_time, and the location of output and wether to record response or not. $# parameters where provided. 4 are needed. Example: '
    echo ""
    echo 'script.sh qap tai35a.dat.dat experiment_results/inter_instance_transfer/qap_tai35a/experiment_1/fittest_1 0.5 results.txt false'
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



cat > tmp.ini <<EOF
; config file for test in hpc hipatia


[Global] 
MODE = test ;
PROBLEM_NAME = permu


[TestSettings]
THREADS = 32 ;
N_EVALS = 500 ;
N_REPS = 20 ;
CONTROLLER_PATH = $3 ; 
COMPUTE_RESPONSE = $6

[Controller]
MAX_TIME_PSO = $4 ; 
POPSIZE = 20 ;
TABU_LENGTH = 40 ;
N_EVALS = 500 ;
N_REPS = 20 ;

PROBLEM_TYPE = $1 ; 
PROBLEM_PATH = $2 ; 

EOF


date
./neat "tmp.ini"
cat "result.txt" >> "$SRCDIR/$5"
cat "responses.txt" >> "$SRCDIR/src/experiments/permus/results/analyze_outputs/responses_journal.txt"


date


# #end