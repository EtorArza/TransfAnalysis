#!/bin/bash

# https://stackoverflow.com/questions/17403498/iterate-over-two-arrays-simultaneously-in-bash

srun bash scripts/make_hip.sh

array1=($(ls src/experiments/permus/instances/qap/cut_instances/cut30*0b)) # 0.1s 
array2=($(ls src/experiments/permus/instances/qap/cut_instances/cut30*0a)) # 0.1
array3=($(ls src/experiments/permus/instances/qap/cut_instances/cut30*sko*)) # 0.1

array4=($(ls src/experiments/permus/instances/qap/cut_instances/cut60*0b)) # 0.3
array5=($(ls src/experiments/permus/instances/qap/cut_instances/cut60*0a)) # 0.3
array6=($(ls src/experiments/permus/instances/qap/cut_instances/cut60*sko*)) # 0.3

array7=($(ls src/experiments/permus/instances/qap/cut_instances/cut30*0a)) # 0.1
array8=($(ls src/experiments/permus/instances/qap/cut_instances/cut60*0a)) # 0.3

array9=($(ls src/experiments/permus/instances/qap/cut_instances/cut30*sko*)) # 0.1
array10=($(ls src/experiments/permus/instances/qap/cut_instances/cut60*sko*)) # 0.3

array11=($(ls src/experiments/permus/instances/qap/cut_instances/cut30*0b)) # 0.1
array12=($(ls src/experiments/permus/instances/qap/cut_instances/cut60*0b)) # 0.3


script_dir="src/experiments/permus_multi/scripts/"




# FIX SIZE
# Size = 30, all together 

for i in 1 2 3 4 5 6 7 8 9 10 11 12; do

varname="\${array$i[*]}"





eval "array=($varname)"

ins_1=${array[0]}
ins_2=${array[1]}
ins_3=${array[2]}


sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "$ins_1" "0.25" "$ins_2" "0.25"
sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "$ins_1" "0.25" "$ins_3" "0.25"
sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "$ins_2" "0.25" "$ins_3" "0.25"


done


