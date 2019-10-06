for controller in experiments/tai35a_check_overfitting/test_controllers/*; do
    [ -e "$controller" ] || continue
    for instance in instances/qap/*; do
        [ -e "$instance" ] || continue
        sbatch scripts/exec_hipatia_test.sh "qap" "$instance" "$controller" 
    done
done


