#!/bin/bash


for problem in "qap" "tsp" "lop" "pfsp"
for instance in src/experiments/permus/instances/${problem}/*; do
    sbatch scripts/exec_hipatia_train.sh "$problem" "$instance" 0.25
done
done


for instance in src/experiments/permus/instances/qap/cut_instances/*; do
    sbatch scripts/exec_hipatia_train.sh "qap" "$instance" 0.25
done



