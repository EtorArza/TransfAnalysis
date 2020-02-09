#!/bin/bash

bash scripts/make_hip.sh


for controller in "src/experiments/permus/results/4by4_permu_problems/top_controllers/"*; do
    for problem in "qap" "tsp" "pfsp" "lop"; do
        for instance in "src/experiments/permus/instances/${problem}/"*".${problem}"; do
            sbatch src/experiments/permus/scripts/exec_hipatia_test.sh "${problem}" "$instance" "$controller" "0.25" "/dev/null" "true"
            sleep 0.1
        done
    done
done




for controller in "src/experiments/permus/results/transfer_qap_with_cut_instances/top_controllers/"*; do
    for problem in "qap"; do
        for instance in "src/experiments/permus/instances/qap/cut_instances/"*; do
            sbatch src/experiments/permus/scripts/exec_hipatia_test.sh "${problem}" "$instance" "$controller" "0.25" "/dev/null" "true"
            sleep 0.1
        done
    done
done
