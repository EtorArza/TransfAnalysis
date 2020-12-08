
#include <string>
#include "Parameters.h"
#pragma once


namespace REAL_FUNC{

    class params: public base_params{

    public:




        //PARAMETERS TO BE SPECIFIED IN CONFIG FILE//
        std::string prob_name;
        std::string MODE;

        int MAX_SOLVER_FE;
        int SOLVER_POPSIZE;
        bool FULL_MODEL;

        // TRAIN PARAMS TO BE SPECIFIED IN CONFIG FILE//
        std::vector<int>* PROBLEM_INDEX_LIST; 
        std::vector<int>* PROBLEM_DIM_LIST; 
        std::vector<int>* X_LOWER_LIST;
        std::vector<int>* X_UPPER_LIST;
    
        // TEST PARAMS TO BE SPECIFIED IN CONFIG FILE//
        std::string CONTROLLER_PATH;
        int N_REPS;
        int N_EVALS;
        bool PRINT_POSITIONS;
        int PROBLEM_INDEX;
        int PROBLEM_DIM;
        bool COMPUTE_RESPONSE = false;

        double X_LOWER_LIM;
        double X_UPPER_LIM;


    };

};


