rsync -r -avHPe "ssh -p6556" ./* earza@hpc.bcamath.org:/home/earza/accneat/ --exclude-from=".gitignore"
