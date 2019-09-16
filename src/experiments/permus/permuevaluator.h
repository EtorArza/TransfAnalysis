#pragma once



namespace NEAT {

    enum operator_t{
    SWAP = 0,
    EXCH = 1,
    INSERT = 2,
    N_OPERATORS = 3,
    };

    enum sensor_t {
        OPT_SWAP = SWAP, // 1 if local optima for swap neighborhood. 0 else.
        OPT_EXCH = EXCH,
        OPT_INSERT = INSERT,
        RELATIVE_POSITION = 3,
        RELATIVE_TIME = 4,
        DISTANCE = 5,
        SPARSITY = 6,
        R_NUMBER = 7,
        __sensor_N = 8,

    };

    const int N_COEF = 4;
    enum output_t {
        ls_nothing_move = 0,
        O_ID_SWAP = SWAP + 1,
        O_ID_EXCH = EXCH + 1,
        O_ID_INSERT = INSERT + 1,
        accept_or_reject_worse = 4,
        TABU = 5,
        c_momentum = 6,
        c_pers_best = 7,
        c_best_known = 8,
        c_above = 9,
        __output_N = 10,
    };

    class NetworkEvaluator *create_permu_evaluator();
}
