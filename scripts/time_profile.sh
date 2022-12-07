
cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=false
CFLAGS=-fopenmp -std=c++11 -pthread ‑mveclibabi=svml


PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF
make
rm callgrind.out*

valgrind --tool=callgrind --trace-children=yes --dump-instr=yes --collect-jumps=yes ./main.out test "qap" "src/experiments/permus/instances/qap/tai35a.dat.dat" "fittest_20"

#valgrind --tool=callgrind --trace-children=yes ./main.out test "qap" "src/experiments/permus/instances/qap/tai35a.dat.dat" "fittest_20"


kcachegrind callgrind.out*
