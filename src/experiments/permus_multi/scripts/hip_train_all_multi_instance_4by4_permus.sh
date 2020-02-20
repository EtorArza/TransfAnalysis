#!/bin/bash

# https://stackoverflow.com/questions/17403498/iterate-over-two-arrays-simultaneously-in-bash

srun bash scripts/make_hip.sh

array1=($(ls src/experiments/permus/instances/qap/*.qap)) 
array2=($(ls src/experiments/permus/instances/tsp/*.tsp)) 
array3=($(ls src/experiments/permus/instances/pfsp/*.pfsp)) 
array4=($(ls src/experiments/permus/instances/lop/*.lop)) 




script_dir="src/experiments/permus_multi/scripts/"




# FIX SIZE
# Size = 30, all together 

for i in 0 1 2 3; do

sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array1[i]}" "0.25" "${array2[i]}" "0.25"
sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array1[i]}" "0.25" "${array3[i]}" "0.25"
sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array1[i]}" "0.25" "${array4[i]}" "0.25"
sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array2[i]}" "0.25" "${array3[i]}" "0.25"
sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array2[i]}" "0.25" "${array4[i]}" "0.25"
sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array3[i]}" "0.25" "${array4[i]}" "0.25"



done


