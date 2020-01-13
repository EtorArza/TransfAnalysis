#pragma once




namespace PERMU{

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
        ORDER_SPARSITY = 7,
        __sensor_N = 8,

    };

    const int N_PERMU_REFS = 5;
    enum output_t {
        ls_nothing_move = 0,
        O_ID_SWAP = SWAP + 1,
        O_ID_EXCH = EXCH + 1,
        O_ID_INSERT = INSERT + 1,
        accept_or_reject_worse = 4,
        TABU = 5,
        c_hamming_consensus = 6,
        c_kendall_consensus = 7,
        c_pers_best = 8,
        c_best_known = 9,
        c_above = 10,
        __output_N = 11,
    };
}

namespace NEAT{

class NetworkEvaluator *create_permu_evaluator();

}
