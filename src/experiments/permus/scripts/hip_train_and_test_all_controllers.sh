#!/bin/bash

srun bash scripts/make_hip.sh



JOB_IDS_PROBLEMS=(0 0 0 0)


i=0
for PROBLEM_TYPE in "qap" "tsp" "lop" "pfsp"; do
    INSTANCES="src/experiments/permus/instances/${PROBLEM_TYPE}/*.${PROBLEM_TYPE}"
    NUM_INSTANCES=$( ls -1 $INSTANCES | wc -l)
    NUM_INSTANCES=$(($NUM_INSTANCES - 1))
    ID=`sbatch --export=PROBLEM_TYPE=$PROBLEM_TYPE,INSTANCES=$INSTANCES,MAX_SOLVER_TIME=0.25,N_EVALS=30,DESTINATION_FOLDER=src/experiments/permus/results/4by4_permu_problems/all_controllers --array=0-$NUM_INSTANCES src/experiments/permus/scripts/hip_train_array.sl`
    JOB_IDS_PROBLEMS[i]=${ID##* }
    i=$((i + 1))
done




INSTANCES="src/experiments/permus/instances/qap/cut_instances/*"
NUM_INSTANCES=$( ls -1 $INSTANCES | wc -l)
NUM_INSTANCES=$(($NUM_INSTANCES - 1))
ID=`sbatch --export=PROBLEM_TYPE="qap",INSTANCES=$INSTANCES,MAX_SOLVER_TIME=0.25,N_EVALS=30,SAMPLE_SIZE_UPDATE_BK=9000,N_REEVALS_TOP_5_PERCENT=300,DESTINATION_FOLDER=src/experiments/permus/results/transfer_qap_with_cut_instances/all_controllers --array=0-$NUM_INSTANCES src/experiments/permus/scripts/hip_train_array.sl`
ID_2=${ID##* }



cat > script_f60494109a40e623a82d5c76070b160e.sh <<EOF
#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%A_%a_out.txt
#SBATCH --error=out/slurm_%A_%a_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=2G
#SBATCH --cpus-per-task=2 # number of CPUs
#SBATCH --time=0-00:30:00 #Walltime
#SBATCH -p short

SCRATCH_JOB=${SCRATCH_JOB}_${SLURM_ARRAY_TASK_ID}
mkdir ${SCRATCH_JOB}
module load GCC/8.3.0


bash src/experiments/permus/scripts/hip_test_all_controllers.sh "false"

rm script_f60494109a40e623a82d5c76070b160e.sh
EOF

chmod +x script_f60494109a40e623a82d5c76070b160e.sh

sbatch --dependency=afterok:${JOB_IDS_PROBLEMS[0]}:${JOB_IDS_PROBLEMS[1]}:${JOB_IDS_PROBLEMS[2]}:${JOB_IDS_PROBLEMS[3]}:$ID_2 script_f60494109a40e623a82d5c76070b160e.sh





