
#include <string>
#pragma once


namespace PERMU{

    class params{

    public:
        // PARAMETERS TO BE SPECIFIED IN CONFIG FILE//
        double MAX_TIME_PSO;
        int POPSIZE;
        int N_EVALS;
        int N_REEVALS_TOP_5_PERCENT;
        int N_EVALS_TO_UPDATE_BK;

        int TABU_LENGTH;
        std::string INSTANCE_PATH;
        std::string PROBLEM_TYPE;

        // TEST PARAMS TO BE SPECIFIED IN CONFIG FILE//
        std::string CONTROLLER_PATH;
        int N_REPS;
        bool compute_response = false;
        /////////////////////////////////////////////

    };

};