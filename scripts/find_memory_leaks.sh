cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=true
CFLAGS=-fopenmp -std=c++11 -pthread

N_THREADS=70
PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF
make
valgrind   --leak-check=yes ./neat -f -n 20 permu -t 1 -x 5
