
#include <string>
#include "Parameters.h"
#pragma once



namespace PERMU{

    class params: public base_params{

    public:

        // PARAMETERS TO BE SPECIFIED IN CONFIG FILE//
        double MAX_SOLVER_TIME;
        long MAX_SOLVER_FE;
        int POPSIZE;
        int N_EVALS;
        int TABU_LENGTH;
        std::string INSTANCE_PATH;
        std::string PROBLEM_TYPE;

        // TEST PARAMS TO BE SPECIFIED IN CONFIG FILE//
        std::string CONTROLLER_PATH;
        int N_REPS;
        bool COMPUTE_RESPONSE = false;
        /////////////////////////////////////////////

        std::string prob_name;
        std::string MODE;
        uint32_t SEED;

    };

};