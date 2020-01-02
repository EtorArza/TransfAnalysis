for controller in experiments/transfer_qap_with_cut_instances/top_controllers/*; do
    [ -e "$controller" ] || continue
    for instance in experiments/transfer_qap_with_cut_instances/instances/cut_instances/*; do
        [ -e "$instance" ] || continue
        sbatch scripts/exec_hipatia_test.sh "qap" "$instance" "$controller" 
    done
done


