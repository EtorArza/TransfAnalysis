#pragma once



namespace NEAT {

    enum sensor_t {
        sensor_relat_f = 0,
        sensor_time = 1,
        sensor_rand = 2,
        sensor_relat_spars = 3,
        sensor_is_local_optima = 4,
        sensor_ham_dist_from_theoneabove = 5,
        __sensor_N = 6
    };

    enum output_t {
        ls_nothing_move = 0,
        c_momentum = 1,
        c_pers_best = 2,
        c_best_known = 3,
        c_momentum = 4,
        c_above = 5,
        o_id_exch = 6,
        o_id_swap = 7,
        o_id_insrt = 8,

        __output_N = 9
    };

    class NetworkEvaluator *create_permu_evaluator();
}
