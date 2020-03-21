
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
        int SAMPLE_SIZE_UPDATE_BK;
        int N_SAMPLES_UPDATE_BK;
        int UPDATE_BK_EVERY_K_ITERATIONS;
        
        int TABU_LENGTH;
        std::string INSTANCE_PATH;
        std::string PROBLEM_TYPE;

        // TEST PARAMS TO BE SPECIFIED IN CONFIG FILE//
        std::string CONTROLLER_PATH;
        int N_REPS;
        bool COMPUTE_RESPONSE = false;
        /////////////////////////////////////////////

        double* bk_f_average_sample;
        std::string prob_name;
        std::string MODE;
        int SEED;

    };

};