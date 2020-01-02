for n in 30 60; do
for instance in instances/*; do
    inst_name=$(basename $instance ".dat.dat")
    python cut_instance.py $instance "instances/cut_instances/cut${n}_${inst_name}" $n
done
done