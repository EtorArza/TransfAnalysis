#pragma once

#define EVALS_TO_SELECT_BEST_CONTROLLER_IN_LAST_IT 1000
#define MAX_EVALS_PER_CONTROLLER_NEUROEVOLUTION 100
#define EVAL_MIN_STEP 32

#define MAX(A, B) ((A > B) ? A : B)
#define MIN(A, B) ((A < B) ? A : B)

#define SMALLEST_POSITIVE_DOUBLE  0.0000001

#define TEMP_double_ARRAY_SIZE 30


#define DEPTH_OF_ORDER_MARGINAL 4
#define COMPUTE_ORDER_MARGINAL_EVERY_K_ITERATIONS 6


#define ALPHA_INDEX 0


#if ALPHA_INDEX == 0
    #define ALPHA 0.05
    #define Z_THRESH 1.645
#elif ALPHA_INDEX == 1
    #define ALPHA 0.01
    #define Z_THRESH 2.326
#elif ALPHA_INDEX == 2
    #define ALPHA 0.005
    #define Z_THRESH 2.576
#else
    #error Only indexes 0, 1 and 2 allowed in alpha selection.
#endif



#define CUTOFF_0 0.25


#define MAX_POPSIZE 40
#define MIN_POPSIZE 4

#define MAX_TABU_LENGTH 100
#define MIN_TABU_LENGTH 5
#define LENGTH_CHANGE_STEP 5