#!/bin/bash

#bash scripts/release_compile.sh
make

RES_DIR_1="src/experiments/permus/results/4by4_permu_problems/result_controllers_journal_version_local.txt"

for controller in "src/experiments/permus/results/4by4_permu_problems/top_controllers/"*; do
    for problem in "qap" "tsp" "pfsp" "lop"; do
        for instance in "src/experiments/permus/instances/${problem}/"*".${problem}"; do
#            bash src/experiments/permus/scripts/exec_local_test.sh "${problem}" "$instance" "$controller" "0.25" "${RES_DIR_1}" "false"
#            sleep 0.05
                echo "."
        done
    done
done



RES_DIR_2="src/experiments/permus/results/transfer_qap_with_cut_instances/result_controllers_journal_version_local.txt"
for controller in "src/experiments/permus/results/transfer_qap_with_cut_instances/top_controllers/"*; do
    for problem in "qap"; do
        for instance in "src/experiments/permus/instances/qap/cut_instances/"*; do
            bash src/experiments/permus/scripts/exec_local_test.sh "${problem}" "$instance" "$controller" "0.25" "${RES_DIR_2}" "false"
            sleep 0.05
        done
    done
done
