cat > Makefile.conf <<EOF
DEVMODE=true
CFLAGS= -std=c++11 -pthread -Wall
PARALELLIZE=false

HIPATIA=false
PFM_LD_FLAGS=
PFM_NVCC_CCBIN=
EOF

make


