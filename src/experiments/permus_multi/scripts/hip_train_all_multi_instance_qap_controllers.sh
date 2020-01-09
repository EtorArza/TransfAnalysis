
# https://stackoverflow.com/questions/17403498/iterate-over-two-arrays-simultaneously-in-bash

array1=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut30*0b))
array2=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut30*0a))
array3=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut30*sko*))

array4=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut60*0b))
array5=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut60*0a))
array6=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut60*sko*))

array7=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut30*0a))
array8=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut60*0a))

array9=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut30*sko*))
array10=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut60*sko*))

array11=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut30*0b))
array12=($(ls experiments/transfer_qap_with_cut_instances/instances/cut_instances/cut60*0b))



for index in ${!array1[*]}; do 
    sbatch scripts/exec_hipatia_train_multi.sh "qap" 3 "${array1[$index]}" "${array2[$index]}" "${array3[$index]}"
done


for index in ${!array4[*]}; do 
    sbatch scripts/exec_hipatia_train_multi.sh "qap" 3 "${array4[$index]}" "${array5[$index]}" "${array6[$index]}"
done




for index in ${!array7[*]}; do 
    sbatch scripts/exec_hipatia_train_multi.sh "qap" 2 "${array7[$index]}" "${array8[$index]}"
done


for index in ${!array9[*]}; do 
    sbatch scripts/exec_hipatia_train_multi.sh "qap" 2 "${array9[$index]}" "${array10[$index]}"
done


for index in ${!array11[*]}; do 
    sbatch scripts/exec_hipatia_train_multi.sh "qap" 2 "${array11[$index]}" "${array12[$index]}"
done


