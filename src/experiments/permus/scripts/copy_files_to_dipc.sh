if [[ $# -eq 0 ]] ; then
    echo 'The name of the folder to copy to on hipatia not provided. Exiting...'
    exit 1
fi


mkdir err
mkdir out
#ssh -L 8888:atlas.sw.ehu.es:22 earza@ac-01.sw.ehu.es &
#rsync -r -avHPe "ssh -A earza@ac-01.sw.ehu.es ssh" ./* "earza@atlas.sw.ehu.es:/scratch/earza" --exclude-from=".gitignore"

rsync -r -auve "ssh -p 8888" ./* "paran@localhost:/scratch/earza" --exclude-from=".gitignore"

# rsync -r -avHPe "ssh -L 8888:atlas.sw.ehu.es:22" ./* "earza@ac-01.sw.ehu.es:/scratch/earza" --exclude-from=".gitignore"
rm -r err
rm -r out
