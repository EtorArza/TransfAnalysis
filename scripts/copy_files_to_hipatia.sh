

mkdir err
mkdir out
rsync -r -avHPe "sshpass -p eezA3vEscDbW ssh -p6556" ./* "earza@hpc.bcamath.org:/home/earza/accneat_permus" --exclude-from=".gitignore"
rm -r err
rm -r out


