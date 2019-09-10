cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=false
CFLAGS=-fopenmp -std=c++11 -pthread -Wall -DNDEBUG

PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF

make




./neat test "qap" "src/experiments/permus/instances/qap/tai35a.dat.dat" "fittest_300"
