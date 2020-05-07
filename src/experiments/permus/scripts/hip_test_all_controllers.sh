#!/bin/bash

#bash scripts/make_hip.sh

source scripts/array_to_string_functions.sh

if [ "$#" -ne 1 ]; then
    echo "You must enter exactly 1 command line argument"
    exit 1
fi

MEASURE_RESPONSES=$1


### 4by4 peremuproblems
CONTROLLERS=()
PROBLEMS=()
INSTANCES=()
SRCDIR=`pwd`
SCORE_PATH="src/experiments/permus/results/4by4_permu_problems/result_controllers_journal_version.txt"
RESPONSE_PATH="src/experiments/permus/results/analyze_outputs/responses_journal_4by4_permuproblems.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname $SCORE_PATH)
NUM_JOBS=-1
for controller in "src/experiments/permus/results/4by4_permu_problems/top_controllers/"*; do
    for problem in "qap" "tsp" "pfsp" "lop"; do
        for instance in "src/experiments/permus/instances/${problem}/"*".${problem}"; do
            CONTROLLERS+=($controller)
            PROBLEMS+=($problem)
            INSTANCES+=($instance)
            NUM_JOBS=$((NUM_JOBS + 1))
        done
    done
done


cat > script_527cff9ed08e301393afd8d723ce0182.sh <<EOF
#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j_out.txt
#SBATCH --error=out/slurm_%j_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=2G
#SBATCH --cpus-per-task=2 # number of CPUs
#SBATCH --time=0-00:15:00 #Walltime
#SBATCH -p short


if [ $MEASURE_RESPONSES == "true" ]; then
    cat ${TMP_RES_PATH}/response_* > ${RESPONSE_PATH}
elif [ $MEASURE_RESPONSES == "false" ]; then
    cat ${TMP_RES_PATH}/score_* > ${SCORE_PATH}
else
    echo "MEASURE_RESPONSES = $MEASURE_RESPONSES not set correctly"
    exit(1)
fi


rm script_527cff9ed08e301393afd8d723ce0182.sh
EOF


CONTROLLERS=$(to_list "${CONTROLLERS[@]}")
PROBLEMS=$(to_list "${PROBLEMS[@]}")
INSTANCES=$(to_list "${INSTANCES[@]}")



ID=`sbatch --parsable --export=CONTROLLERS=${CONTROLLERS},PROBLEMS=${PROBLEMS},INSTANCES=${INSTANCES},MAX_SOLVER_TIME=0.25,MEASURE_RESPONSES=${MEASURE_RESPONSES},TMP_RES_PATH=${TMP_RES_PATH} --array=0-$NUM_JOBS src/experiments/permus/scripts/hip_test_array.sl`
sbatch --dependency=afterok:$ID script_527cff9ed08e301393afd8d723ce0182.sh




### QAP cut transfer
CONTROLLERS=()
PROBLEMS=()
INSTANCES=()
SCORE_PATH="src/experiments/permus/results/transfer_qap_with_cut_instances/result_controllers_journal_version.txt"
RESPONSE_PATH="src/experiments/permus/results/analyze_outputs/responses_journal_qap_cut.txt"
TMP_RES_PATH=${SRCDIR}/"tmp"/$(dirname $SCORE_PATH)
NUM_JOBS=-1
for controller in "src/experiments/permus/results/transfer_qap_with_cut_instances/top_controllers/"*; do
    for problem in "qap"; do
        for instance in "src/experiments/permus/instances/qap/cut_instances/"*; do
            CONTROLLERS+=($controller)
            PROBLEMS+=($problem)
            INSTANCES+=($instance)
            NUM_JOBS=$((NUM_JOBS + 1))
        done
    done
done


cat > script_2828a8741ae82e71b77975df5ec94c25.sh <<EOF
#!/bin/bash
###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%i_out.txt
#SBATCH --error=out/slurm_%i_err.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=2G
#SBATCH --cpus-per-task=2 # number of CPUs
#SBATCH --time=0-00:15:00 #Walltime
#SBATCH -p short

if [ $MEASURE_RESPONSES == "true" ]; then
    cat ${TMP_RES_PATH}/response_* > ${RESPONSE_PATH}
elif [ $MEASURE_RESPONSES == "false" ]; then
    cat ${TMP_RES_PATH}/score_* > ${SCORE_PATH}
else
    echo "MEASURE_RESPONSES = $MEASURE_RESPONSES not set correctly"
    exit(1)
fi

rm script_2828a8741ae82e71b77975df5ec94c25.sh
EOF

CONTROLLERS=$(to_list "${CONTROLLERS[@]}")
PROBLEMS=$(to_list "${PROBLEMS[@]}")
INSTANCES=$(to_list "${INSTANCES[@]}")


ID=`sbatch --parsable --export=CONTROLLERS=${CONTROLLERS},PROBLEMS=${PROBLEMS},INSTANCES=${INSTANCES},MAX_SOLVER_TIME=0.25,MEASURE_RESPONSES=${MEASURE_RESPONSES},TMP_RES_PATH=${TMP_RES_PATH} --array=0-$NUM_JOBS src/experiments/permus/scripts/hip_test_array.sl`
sbatch --dependency=afterok:$ID script_2828a8741ae82e71b77975df5ec94c25.sh

