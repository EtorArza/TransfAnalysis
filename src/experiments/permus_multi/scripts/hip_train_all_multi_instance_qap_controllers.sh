
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

SMALL_TIME=0.25
BIG_TIME=0.25



# FIX SIZE
# Size = 30, all together 
for index in ${!array1[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 3 "${array1[$index]}" ${SMALL_TIME} "${array2[$index]}" ${SMALL_TIME} "${array3[$index]}" ${SMALL_TIME}
done

# Size = 30, missing sko
for index in ${!array1[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array1[$index]}" ${SMALL_TIME} "${array2[$index]}" ${SMALL_TIME} 
done

# Size = 30, missing a 
for index in ${!array1[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array1[$index]}" ${SMALL_TIME}                           "${array3[$index]}" ${SMALL_TIME}
done

# Size = 30, missing b 
for index in ${!array1[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2                           "${array2[$index]}" ${SMALL_TIME} "${array3[$index]}" ${SMALL_TIME}
done



# Size = 60, all together 
for index in ${!array4[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 3 "${array4[$index]}" ${BIG_TIME} "${array5[$index]}" ${BIG_TIME} "${array6[$index]}" ${BIG_TIME}
done

# Size = 60, missing sko 
for index in ${!array4[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array4[$index]}" ${BIG_TIME} "${array5[$index]}" ${BIG_TIME}
done

# Size = 60, missing a
for index in ${!array4[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array4[$index]}" ${BIG_TIME}                           "${array6[$index]}" ${BIG_TIME}
done

# Size = 60, missing b 
for index in ${!array4[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2                           "${array5[$index]}" ${BIG_TIME} "${array6[$index]}" ${BIG_TIME}
done





# FIX TYPE

for index in ${!array7[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array7[$index]}" ${SMALL_TIME} "${array8[$index]}" ${BIG_TIME}
done


for index in ${!array9[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array9[$index]}" ${SMALL_TIME} "${array10[$index]}" ${BIG_TIME}
done


for index in ${!array11[*]}; do 
    sbatch "${script_dir}exec_hipatia_train_multi.sh" "qap" 2 "${array11[$index]}" ${SMALL_TIME} "${array12[$index]}" ${BIG_TIME}
done


