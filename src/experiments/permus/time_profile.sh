g++  -std=c++11 permu_problem_src/*.cpp permu_problem_src/*.h -o binary.out -g
./binary.out "lop" "instances/lop/Cebe.lop.n30.1"


valgrind --tool=callgrind --trace-children=yes ./binary.out "lop" "instances/lop/Cebe.lop.n30.1"


kcachegrind callgrind.out.*
