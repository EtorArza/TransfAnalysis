g++  -std=c++11 permu_problem_src/*.cpp permu_problem_src/*.h -o binary.out -g

valgrind   --leak-check=yes ./binary.out "lop" "instances/lop/Cebe.lop.n30.1";