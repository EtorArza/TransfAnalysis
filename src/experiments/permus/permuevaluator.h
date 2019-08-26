#pragma once


namespace NEAT {

    const int SWAP = 0;
    const int EXCH = 1;
    const int INSERT = 2;

    const int N_OPERATORS = 3;

    enum sensor_t {
        OPT_SWAP = SWAP, // 1 if local optima for swap neighborhood. 0 else.
        OPT_EXCH = EXCH,
        OPT_INSERT = INSERT,
        RELATIVE_POSITION = 3,
        RELATIVE_TIME = 4,
        DISTANCE = 5,
        SPARSITY = 6,
        R_NUMBER = 7,
        __sensor_N = 8
    };

    const int N_COEF = 4;
    enum output_t {
        ls_nothing_move = 0,
        O_ID_SWAP = SWAP + 1,
        O_ID_EXCH = EXCH + 1,
        O_ID_INSERT = INSERT + 1,
        c_momentum = 4,
        c_pers_best = 5,
        c_best_known = 6,
        c_above = 7,
        __output_N = 8
    };

    class NetworkEvaluator *create_permu_evaluator();
}
