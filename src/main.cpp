#define SAME_SIZE_EXPERIMENT


/*
  Copyright 2001 The University of Texas at Austin

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#include "std.h" // Must be included first. Precompiled header with standard library includes.
#include "INIReader.h"
#include <unistd.h>
#include "experiment.h"
#include "neat.h"
#include "rng.h"
#include "util.h"
#include <omp.h>
#include "loadnetworkfromfile.h"
#include "Population.h"
#include "PBP.h"
#include "QAP.h"
#include "LOP.h"
#include "PFSP.h"
#include "TSP.h"
#include "permuevaluator.h"
#include "Tools.h"
#include "FitnessFunction_permu.h"


#define EXTERN
#include "Parameters.h"
#include <iomanip>      // std::setprecision


using namespace NEAT;
using namespace std;



void usage()
{

    cerr << "usage: \n ./neat path_of_config_file" << endl;
    cerr << "for example, \n ./neat \"config_files/test.ini\"" << endl;
    cerr << endl;
    exit(1);
}


PBP *GetProblemInfo(std::string problemType, std::string filename)
{
    PBP *problem;
    if (problemType == "pfsp")
    {
        problem = new PFSP();
    }
    else if (problemType == "tsp")
    {
        problem = new TSP();
    }
    else if (problemType == "qap")
    {
        problem = new QAP();
    }
    else if (problemType == "lop")
    {
        problem = new LOP();
    }
    // else if (problemType == "api")
    //     problem = new API();
    else
    {
         cout << "Wrong problem type was specified." << endl;
         exit(1);
     }

    //Read the instance.
    problem->Read_with_mutex(filename);
    #ifdef SAME_SIZE_EXPERIMENT
        if (problem->GetProblemSize() == 30)
        {
            MAX_TIME_PSO = 0.10;
        }
        else if(problem->GetProblemSize() == 60)
        {
            MAX_TIME_PSO = 0.3;
        }
        else
        {
            cout << "ERROR, this experiment expects instances of size 60 and 30.";
            exit(1);
        }

    #undef SAME_SIZE_EXPERIMENT
    #endif
    return problem;
}



int main(int argc, char *argv[])
{   

    cout << "\n\n---------- BEGIN LICENCE DISCLAIMER----------\n";
    cout << "This code is based on accneat. Although some small changes \n";
    cout << "have been made, most of the NEAT algorithm remains unchanged.\n" << endl;
	cout <<	"Accneat is a fork of Stanley et al.'s implementation with some \n";
    cout << "improvements such as delete mutations and speed improvements, available at \n";
    cout << "https://github.com/sean-dougherty/accneat\n" << endl << endl;
    cout << "This code also uses a configuration file parser inih, available at https://github.com/jtilly/inih\n" << endl;
    cout << "The authors of the modified software distributed here are in NO way affiliated with accneat or INIH. Please, \n";
    cout << "understand that the use of this software requires reading and accepting the licences of both accneat and INIH.\n" << endl << endl;
    cout << "INIH is distributed with BSD licence, and accneat with APACHE LICENCE 2.0\n";
    cout << "The source code provided here (excluding accneat and INIH) was made by Etor Arza.\n";
    cout << "To keep the licence stuff as painless as possible, the software part writen by \n";
    cout << "Etor Arza is distributed with APACHE LICENCE 2.0 too.\n\n\n";
    cout << "This modified software contains some parts of the PerMallows package by Ekhiñe Irurozki available at https://cran.r-project.org/web/packages/PerMallows/index.html" << endl;
    cout << "----------END LICENCE DISCLAIMER----------\n\n";

    system("rm -r controllers_trained_with_cut60_tai80b");



    #ifndef NDEBUG
    cout << "WARNING: Debug mode. Assertions enabled." << endl;
    #endif

    if (argc < 2)
    {
        cout << "Error, no configuration file provided.\n";
        exit(1);
    }else if (argc > 2)
    {
        cout << "Error, too many arguments provided. Provide only path to configuration file.";
        exit(1);

    }

    INIReader reader(argv[1]);

    if (reader.ParseError() != 0) {
        std::cout << "Can't load " << argv[1] << "\n";
        return 1;
    }

    string MODE = reader.Get("Global", "mode", "UNKNOWN");




    if (MODE == "train")
    {


        int rng_seed = reader.GetInteger("NEAT","SEED", -1);
        env->pop_size = reader.GetInteger("NEAT", "POPSIZE", -1);
        int max_time = reader.GetInteger("NEAT", "MAX_TRAIN_TIME", -1);
        N_OF_THREADS = reader.GetInteger("NEAT", "THREADS", -1);
        N_EVALS = reader.GetInteger("NEAT", "N_EVALS", -1);
        N_REEVALS_TOP_5_PERCENT = reader.GetInteger("NEAT","N_REEVALS_TOP_5_PERCENT", -1);
        N_EVALS_TO_UPDATE_BK = reader.GetInteger("NEAT","N_EVALS_TO_UPDATE_BK", -1);
        string search_type = reader.Get("NEAT", "SEARCH_TYPE", "UNKOWN");
        PROBLEM_TYPE = reader.Get("Controller", "PROBLEM_TYPE", "UNKOWN");
        INSTANCE_PATH = reader.Get("Controller", "PROBLEM_PATH", "UNKOWN");
        MAX_TIME_PSO = reader.GetReal("Controller", "MAX_TIME_PSO", -1.0);
        POPSIZE = reader.GetInteger("Controller", "POPSIZE", -1);
        TABU_LENGTH = reader.GetInteger("Controller", "TABU_LENGTH", -1);
        START_WITHOUT_HIDDEN = reader.GetBoolean("NEAT","START_WITHOUT_HIDDEN", false);

        EXPERIMENT_FOLDER_NAME = "controllers_trained_with_" + from_path_to_filename(INSTANCE_PATH);

        cout << "Learning from instance: " << from_path_to_filename(INSTANCE_PATH) << endl;


        F_VALUES_OBTAINED_BY_BEST_INDIV = new double[N_EVALS_TO_UPDATE_BK];
        for (int i = 0; i < N_EVALS_TO_UPDATE_BK; i++)
        {
            F_VALUES_OBTAINED_BY_BEST_INDIV[i] = -DBL_MAX;
        }
        


        if (search_type == "phased")
        {
            env->search_type = GeneticSearchType::PHASED;
        }
        else if (search_type == "blended")
        {
            env->search_type = GeneticSearchType::BLENDED;
        }
        else if (search_type == "complexify")
        {
            env->search_type = GeneticSearchType::COMPLEXIFY;
        }
        else
        {
            cout << "Error, no search type specified." << endl;
        }

        if(N_OF_THREADS < 0){
            cout << "please specify a valid number of threads on the conf. file" <<endl;
            exit(1);
        }


        else if (exists(EXPERIMENT_FOLDER_NAME))
        {
            error("Already exists: " + EXPERIMENT_FOLDER_NAME + ".\nMove your experiment directories or use -f to delete them automatically. If -f is used, all previous experiments will be deleted.")
        }

        omp_set_num_threads(N_OF_THREADS);

        if (N_OF_THREADS < 7)
        {
            cout << "Warning: a minimum of 7 cores (specified by the THREADS parameter) is recommended for this implementation of NEAT to function correctly." << endl;
            cout << "With the current settings, processing a generation takes around " ; 
            cout << ((env->pop_size*MAX_TIME_PSO / (double)2 * (double)N_EVALS + (double)(5/N_OF_THREADS + 1) * (double)N_REEVALS_TOP_5_PERCENT)) / 3600.0; 
            cout << "h,  which might be too long." << endl << endl;
        }

        if (env->pop_size < 500)
        {
            cout << "Warning: The population size of the controllers might be too low." << endl;
            cout << "The provided population size of the controllers is " << env->pop_size << ", a value of at least 500 is recommended." << endl;
            cout << endl << endl;
        }



        MAX_TRAIN_TIME = max_time; 

        if (env->search_type == GeneticSearchType::BLENDED)
        {
            env->mutate_delete_node_prob *= 0.1;
            env->mutate_delete_link_prob *= 0.1;
        }





        const char * prob_name = "permu";
        Experiment *exp = Experiment::get(prob_name);
        rng_t rng{rng_seed};
        global_timer.tic();
        exp->run(rng);

        delete[] F_VALUES_OBTAINED_BY_BEST_INDIV;

        return 0;
    }
    else if (MODE ==  "test")
    {

        //const char * prob_name = "permu";
        //Experiment *exp = Experiment::get(prob_name);

        PROBLEM_TYPE = reader.Get("Controller", "PROBLEM_TYPE", "UNKOWN");
        INSTANCE_PATH = reader.Get("Controller", "PROBLEM_PATH", "UNKOWN");
        MAX_TIME_PSO = reader.GetReal("Controller", "MAX_TIME_PSO", -1.0);
        POPSIZE = reader.GetInteger("Controller", "POPSIZE", -1);
        TABU_LENGTH = reader.GetInteger("Controller", "TABU_LENGTH", -1);
        CONTROLLER_PATH = reader.Get("TestSettings", "CONTROLLER_PATH", "UNKNOWN");
        N_REPS = reader.GetInteger("TestSettings", "N_REPS", -1);
        N_EVALS = reader.GetInteger("TestSettings", "N_EVALS", -1);
        N_OF_THREADS = reader.GetInteger("TestSettings", "THREADS", 1);
        N_OF_THREADS = min(N_OF_THREADS, N_EVALS);


        if (CONTROLLER_PATH == "UNKNOWN")
        {
            cout << "error, controller path not specified in test." << endl;
        }

        if (N_REPS < 0)
        {
             cout << "error, N_REPS not provided in test mode." << endl;
        }
        
        
        CpuNetwork net = load_network(CONTROLLER_PATH);

        double *v_of_f_values = new double[N_EVALS];

        cout << std::setprecision(15);
        RandomNumberGenerator* rng;
        rng = new RandomNumberGenerator();
        rng->seed();
        int initial_seed = rng->random_integer_uniform(40000000, 50000000);
        delete rng;
        cout << "[[";
        for (int j = 0; j < N_REPS; j++)
        {
            #pragma omp parallel for num_threads(N_OF_THREADS)
            for (int i = 0; i < N_EVALS; i++)
            {
                v_of_f_values[i] = FitnessFunction_permu(&net, 1, initial_seed + i);
            }
            initial_seed += N_EVALS;
            double res = Average(v_of_f_values, N_EVALS);
            cout << res;
            if (j < N_REPS-1)
            {
                cout << ",";
            }
            
        }
        cout << "]," << std::flush;
;
        delete[] v_of_f_values;


        

        cout << std::setprecision(15);
        cout << std::flush;
        cout << INSTANCE_PATH   << ","
             << CONTROLLER_PATH << ","
             << PROBLEM_TYPE    << ","
             << N_EVALS
             << "]"
             << endl;


        // cout << res << std::endl;;
        cout << std::flush;

        return 0;
    }
    else
    {
        cout << "invalid mode provided. Please, use the configuration file to specify either test or train." << endl;
        exit(1);
    }
}
