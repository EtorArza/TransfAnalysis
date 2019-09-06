
cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=false
CFLAGS=-fopenmp -std=c++11 -pthread ‑mveclibabi=svml


PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF
make
rm callgrind.out*

valgrind --tool=callgrind --trace-children=yes --dump-instr=yes --collect-jumps=yes ./neat -f -n 20 permu -t 1 -x 5

#valgrind --tool=callgrind --trace-children=yes ./neat -f -n 20 permu -t 1 -x 5


kcachegrind callgrind.out*
