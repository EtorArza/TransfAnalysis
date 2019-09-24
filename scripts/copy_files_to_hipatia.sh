if [[ $# -eq 0 ]] ; then
    echo 'The name of the folder to copy to on hipatia not provided. Exiting...'
    exit 1
fi


mkdir err
mkdir out
rsync -r -avHPe "ssh -p6556" ./* "earza@hpc.bcamath.org:/home/earza/$1" --exclude-from=".gitignore"
rm -r err
rm -r out
