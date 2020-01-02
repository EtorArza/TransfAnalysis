cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=true
CFLAGS=-fopenmp -std=c++11 -pthread


PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF
make
valgrind   --leak-check=yes ./neat "train" -f -n 20 -t 2 -x 4
