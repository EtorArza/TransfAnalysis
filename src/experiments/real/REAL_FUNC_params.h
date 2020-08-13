
#include <string>
#pragma once


namespace REAL_FUNC{

    class params{

    public:

        neat_parameters *neat_params;



        //PARAMETERS TO BE SPECIFIED IN CONFIG FILE//
        std::string prob_name;
        std::string MODE;
        int SEED;

        double MAX_SOLVER_FE;
        int SOLVER_POPSIZE;
        
        // TRAIN PARAMS TO BE SPECIFIED IN CONFIG FILE//
        std::vector<int>* PROBLEM_INDEX_LIST; 
        std::vector<int>* PROBLEM_DIM_LIST; 

        // TEST PARAMS TO BE SPECIFIED IN CONFIG FILE//
        std::string CONTROLLER_PATH;
        int N_REPS;
        int N_EVALS;
        bool PRINT_POSITIONS;
        int PROBLEM_INDEX;
        int PROBLEM_DIM;
        bool COMPUTE_RESPONSE = false;



    };

};


