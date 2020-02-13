#!/bin/bash

bash scripts/make_hip.sh





RES_DIR_2="src/experiments/permus_multi/results/result_controllers_cut_qap_multi_on_journal_version.txt"

# test controllers
for controller in "src/experiments/permus_multi/results/top_controllers/"*; do
    for problem in "qap"; do
        for instance in "src/experiments/permus/instances/qap/cut_instances/"*; do
            sbatch src/experiments/permus/scripts/exec_hipatia_test.sh "${problem}" "$instance" "$controller" "0.25" "${RES_DIR_2}" "false"
            sleep 0.1
        done
    done
done


# analyze outputs
for controller in "src/experiments/permus_multi/results/top_controllers/"*; do
    for problem in "qap"; do
        for instance in "src/experiments/permus/instances/qap/cut_instances/"*; do
            sbatch src/experiments/permus/scripts/exec_hipatia_test.sh "${problem}" "$instance" "$controller" "0.25" "./" "true"
            sleep 0.1
        done
    done
done
