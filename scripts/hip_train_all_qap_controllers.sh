    for instance in instances/qap/*; do
        [ -e "$instance" ] || continue
        sbatch scripts/exec_hipatia_train.sh "qap" "$instance"
    done



