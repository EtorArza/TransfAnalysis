for n in 40; do
for instance in /home/paran/Dropbox/BCAM/QAPinstances/transfer_qap_NEAT/*; do
    inst_name=$(basename $instance ".dat.dat")
    python cut_instance.py $instance "cut_instances/cut${n}_${inst_name}" $n
done
done
