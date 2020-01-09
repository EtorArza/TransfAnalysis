#include <string>
#include "../../PERMU_params.h"
#pragma once


namespace PERMU_MULTI{

    class params_multi: public PERMU::params{

    public:
        int N_OF_INSTANCES;
        std::string* INSTANCE_PATHS;

        double* MAX_TIME_PSO_FOR_EACH_INSTANCE;
        double* BEST_FITNESS_TRAIN_FOR_EACH_INSTANCE;
        double** F_VALUES_OBTAINED_BY_BEST_INDIV_FOR_EACH_INSTANCE; // [which_instance][which_run]
    };

};