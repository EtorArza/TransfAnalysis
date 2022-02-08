#pragma once
#include <limits.h>

#define MAX_EVALS_PER_CONTROLLER 400
#define MAX_EVALS_PER_CONTROLLER_REEVAL 5000


#define MAX(A, B) ((A > B) ? A : B)
#define MIN(A, B) ((A < B) ? A : B)

#define SMALLEST_POSITIVE_DOUBLE  10e-70

#define TEMP_double_ARRAY_SIZE 30


#define DEPTH_OF_ORDER_MARGINAL 4
#define COMPUTE_ORDER_MARGINAL_EVERY_K_ITERATIONS 0





#define CUTOFF_0 0.25


#define MAX_POPSIZE 10
#define MIN_POPSIZE 10

#define MAX_TABU_LENGTH 100
#define MIN_TABU_LENGTH 5
#define LENGTH_CHANGE_STEP 5