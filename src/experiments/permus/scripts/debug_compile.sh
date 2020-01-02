cat > Makefile.conf <<EOF
ENABLE_CUDA=false
DEVMODE=true
CFLAGS=-fopenmp -std=c++11 -pthread -Wall

PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF

make


