#pragma once


 
namespace NEAT {

    // enum operator_t{
    // SWAP = 0,
    // EXCH = 1,
    // INSERT = 2,
    // N_OPERATORS = 3,
    // };

    enum sensor_t {
        DIST_TO_CLOSEST = 0,
        DIST_TO_AVERAGE = 1,
        RELATIVE_DIST_TO_BEST = 2,
        RELATIVE_TIME = 3,
        RELATIVE_SCORE = 4,
        INDIVIDUAL_BEST_WAS_IMPROVED = 5,
        GLOBAL_BEST_WAS_IMPROVED = 6,
        RANDOM_NUMBER = 7,
        __sensor_N = 8,
    };

    enum output_t {
        MOMENTUM = 0,
        G_BEST = 1,
        L_BEST = 2,
        __output_N = 3,
    };

    class NetworkEvaluator *create_real_func_evaluator();
}
