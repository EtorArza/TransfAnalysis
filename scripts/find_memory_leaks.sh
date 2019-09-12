cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=true
CFLAGS=-fopenmp -std=c++11 -pthread

N_THREADS=70
PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF
make
valgrind   --leak-check=yes ./neat "train" -f -n 20 -2 -x 4
