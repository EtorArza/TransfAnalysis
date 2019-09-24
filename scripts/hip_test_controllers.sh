#!/bin/bash


###   s b a t c h --array=1-$runs:1 $SL_FILE_NAME
#SBATCH --output=out/slurm_%j.txt
#SBATCH --error=err/slurm_err_%j.txt
#SBATCH --ntasks=1 # number of tasks
#SBATCH --ntasks-per-node=1 #number of tasks per node
#SBATCH --mem=10G
#SBATCH --cpus-per-task=14 # number of CPUs
#SBATCH --time=0-12:00:00 #Walltime
#SBATCH -p large
###SBATCH --exclude=n[001-016]



SRCDIR=`pwd`


## ls

cp ./* -v -r $SCRATCH_JOB
# mkdir $SCRATCH_JOB/data
# cp $dsname -v $SCRATCH_JOB/data
cd $SCRATCH_JOB

# echo `pwd`
# echo `ls`
# echo `ls data`
module load Python/3.6.1-iomkl-2016a
##module load Tk/8.6.4-foss-2016a-no-X11
pip install --user joblib
##pip install --user numba
##pip install --user neat-python
##pip install --user graphviz
##pip install --user matplotlib


cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=false
CFLAGS=-fopenmp -std=c++11 -pthread

PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF

make


date
python "scripts/test_qap.py"
date
# # # cp best.pickle -v $SRCDIR
# # # cp speciation.png -v $SRCDIR



# #end
