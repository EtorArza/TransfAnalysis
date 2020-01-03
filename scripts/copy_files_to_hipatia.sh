if [[ "$#" -ne 1  ]] ; then
    echo 'Please provide the name folder of the hipatia server. Example:'
    echo ""
    echo 'bash copy_files_to_hipatia.sh accneat_permus'
    echo ""
    echo 'Exitting...'
    exit 1
fi



mkdir err
mkdir out
rsync -r -avHPe "sshpass -p eezA3vEscDbW ssh -p6556" ./* "earza@hpc.bcamath.org:/home/earza/$1" --exclude-from=".gitignore"
rm -r err
rm -r out


