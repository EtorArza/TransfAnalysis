
#include <string>
#pragma once


namespace REAL_FUNC{

    class params{

    public:
        // PARAMETERS TO BE SPECIFIED IN CONFIG FILE//
        double MAX_EVALS_PSO;
        int POPSIZE;
        int N_EVALS;
        int N_REEVALS_TOP_5_PERCENT;
        int N_EVALS_TO_UPDATE_BK;

        int TABU_LENGTH;
        int PROBLEM_INDEX;
        int PROBLEM_DIMENSIONS;

        // TEST PARAMS TO BE SPECIFIED IN CONFIG FILE//
        std::string CONTROLLER_PATH;
        int N_REPS;
        /////////////////////////////////////////////

    };

};