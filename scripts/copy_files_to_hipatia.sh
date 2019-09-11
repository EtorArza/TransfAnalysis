cd ../

rsync -r -avHPe "ssh -p6556" ./* earza@hpc.bcamath.org:/home/earza/accneat/ --exclude-from="/home/paran/Dropbox/BCAM/02_NEAT_permus/code/accneat/.gitignore"
