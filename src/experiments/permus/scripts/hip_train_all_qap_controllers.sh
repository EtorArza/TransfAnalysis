#!/bin/bash


for problem in "qap" "tsp" "lop" "pfsp"; do
for instance in src/experiments/permus/instances/${problem}/*; do
    sbatch src/experiments/permus/scripts/exec_hipatia_train.sh "$problem" "$instance" 0.25
done
done


for instance in src/experiments/permus/instances/qap/cut_instances/*; do
    sbatch src/experiments/permus/scripts/exec_hipatia_train.sh "qap" "$instance" 0.25
done



