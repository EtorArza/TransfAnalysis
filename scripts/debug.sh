cd ../
cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=true
CFLAGS=-fopenmp -std=c++11 -pthread

N_THREADS=1
PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF

make

date
./neat -f -n 8 permu -t 1 -x 6
date



# #end
