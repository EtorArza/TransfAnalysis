for controller in experiments/transfer_qap/top_controllers/*; do
    [ -e "$controller" ] || continue
    for instance in instances/qap/*; do
        [ -e "$instance" ] || continue
        sbatch scripts/exec_hipatia_test.sh "qap" "$instance" "$controller" 
    done
done


