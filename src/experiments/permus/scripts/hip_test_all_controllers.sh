#!/bin/bash

bash scripts/make_hip.sh

RES_DIR_1="src/experiments/permus/results/4by4_permu_problems/result_controllers_journal_version.txt"

for controller in "src/experiments/permus/results/4by4_permu_problems/top_controllers/"*; do
    for problem in "qap" "tsp" "pfsp" "lop"; do
        for instance in "src/experiments/permus/instances/${problem}/"*".${problem}"; do
            sbatch src/experiments/permus/scripts/exec_hipatia_test.sh "${problem}" "$instance" "$controller" "0.25" "${RES_DIR_1}"
            sleep 0.1
        done
    done
done



RES_DIR_2="src/experiments/permus/results/transfer_qap_with_cut_instances/result_controllers_journal_version.txt"

for controller in "src/experiments/permus/results/transfer_qap_with_cut_instances/top_controllers/"*; do
    for problem in "qap"; do
        for instance in "src/experiments/permus/instances/qap/cut_instances/"*; do
            sbatch src/experiments/permus/scripts/exec_hipatia_test.sh "${problem}" "$instance" "$controller" "0.25" "${RES_DIR_2}"
            sleep 0.1
        done
    done
done
