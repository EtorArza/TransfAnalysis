
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
        int SAMPLE_SIZE_UPDATE_BK;
        int N_SAMPLES_UPDATE_BK;
        int UPDATE_BK_EVERY_K_ITERATIONS;
        
        int TABU_LENGTH;
        int PROBLEM_INDEX;
        int PROBLEM_DIMENSIONS;

        // TEST PARAMS TO BE SPECIFIED IN CONFIG FILE//
        std::string CONTROLLER_PATH;
        int N_REPS;
        bool PRINT_POSITIONS;
        /////////////////////////////////////////////

        double* bk_f_average_sample;


    };

};