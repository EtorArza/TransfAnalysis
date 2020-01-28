#!/bin/bash

for controller in src/experiments/permus/results/transfer_qap_with_cut_instances/top_controllers/*; do
    for instance in src/experiments/permus/instances/qap/cut_instances/*; do
        first_line=$(head -n 1 ${instance})
        if [ ${first_line} == "60" ]
        then
        pso_time=0.3
        elif [ ${first_line} == "30" ]
        then 
        pso_time=0.1
        else
        echo "ERROR, only instances of size 30 or 60 should be trained. The instance"
        echo $instance
        echo " was attempted to be used as test instance."
        fi
        sbatch scripts/exec_hipatia_test.sh "qap" "$instance" "$controller" "$pso_time"
    done
done


