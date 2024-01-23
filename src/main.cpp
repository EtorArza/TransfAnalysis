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
#include <unistd.h>
#include "Tools.h"
#include "FitnessFunction_permu.h"
#include "experiment.h"
#include "rng.h"
#include "evaluatorexperiment.h"
#include "INIReader.h"
#include "Parameters.h"
#include "generator.h"
#include "MultidimBenchmarkFF.h"
#include "PBP.h"
#include "Individual.h"
#include "FitnessFunction_permu.h"

void usage()
{
    using namespace std;
    cerr << endl;
    cerr << "Usage: \n ./main.out path_of_config_file" << endl;
    cerr << "for example, \n ./main.out \"config_files/test.ini\"" << endl;
    cerr << "to delete experiment folder with the same name, use: \n";
    cerr << "\n ./main.out -f \"config_files/test.ini\"" << endl;
    cerr << endl;
    exit(1);
}

int main(int argc, char *argv[])
{

    // evaluate continuous benchmark problems
    if(argc > 2 && std::string(argv[1])=="-evaluate-continuous-problem")
    {
        if (argc < 4)
        {
            cout << "For -evaluate-continuous-problem, the following format is expected to"
            << "compute the fitness of (0.5,0.9,0.1) and (0.2,0.1,0.421) on problem 3. " << endl;
            cout << "./main.out -evaluate-continuous-problem 3 0.5,0.9,0.1 0.2,0.1,0.421" << endl;
        }
        
        int problem_index = atoi(argv[2]);

        for (size_t j = 3; j < argc; j++)
        {
            auto split_genome_string = split_string(argv[j], ",");
            double x[300] = {0};
            int n = split_genome_string.size();
            MultidimBenchmarkFF* prob = load_problem_with_default_lims(problem_index, n, -1, false);
            for (size_t i = 0; i < split_genome_string.size(); i++)
            {
                x[i] = atof(split_genome_string[i].c_str());
            }

            if (problem_index > 0)
            {
                cout << prob->Fitness_Func_0_1(x) << " ";
            }
            else if (problem_index == 0)
            {
                // Rokkonen plot function
                char config_path[] = "src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat";
                g_seeded_initialize(config_path, 501, n);
                cout << g_calculate(x) <<  " ";
            }
        }
        

        exit(0);
       

    }

    // local search permus
    if (std::string(argv[1]) == "-local-search-permus")
    {
        
        if (argc != 3)
        {
            std::cerr << "ERROR: wrong number of input arguments. Example usage: \n\n./main.out -local-search-permus src/experiments/permus/instances/transfer_tsp_instances/eil101.tsp\n" << std::flush;
            exit(1);
        }

        size_t budget = 10000;

        PERMU::PBP *problem;

        std::string problemPath = std::string(argv[2]);
        std::string problemType = problemPath.substr(problemPath.find_last_of('.') + 1);        
        GetProblemInfo(problemType, problemPath, &problem);
        RandomNumberGenerator *rng = new RandomNumberGenerator();

        typedef PERMU::operator_t operator_type;
        operator_type operators[3] = {PERMU::SWAP, PERMU::EXCH, PERMU::INSERT};
        PERMU::CIndividual ind = PERMU::CIndividual(problem->GetProblemSize(), rng);
        problem->Evaluate(&ind);

        std::cout << "[";
        for (size_t operator_idx = 0; operator_idx < 3; operator_idx++)
        {
            operator_type op = operators[operator_idx];
            double f_best = -1e40;
            ind.reset(rng);
            problem->Evaluate(&ind);
            for (size_t i = 0; i < budget; i++)
            {
                problem->local_search_iteration(&ind, op);
                if (f_best < ind.f_value)
                {
                    f_best = ind.f_value;
                }
                
                if (ind.is_local_optimum[op])
                {
                    ind.reset(rng);
                    problem->Evaluate(&ind);
                }
            }
            std::cout << f_best << ",";
        }
        std::cout << "]\n";
        std::cout << std::flush;
        exit(0);
    }

    // int n = 13;

    // int sigma_1[n] = {5,11,2,9,4,6,0,7,8,3,10,1,12};
    // int sigma_2[n] = {12,11,10,9,8,7,6,5,4,3,2,1,0};

    // cout << Kendall(sigma_1, sigma_2, n) / (double) (n*(n-1) / 2) << endl;
    // cout << l1_distance(sigma_1, sigma_2, n) / (double) (n*n / 2) << endl;
    // cout << n*n / 2 << endl;
    // exit(1);


    // F7 rosembrock_test = F7(3, -2.048,2.048);
    // double x[3] = {1.0,1.0,1.2};
    // cout << rosembrock_test.FitnessFunc(x);
    // exit(1);

    // int n = 15;
    // int y = 5;

    // cout << p_sign_test(n,y) << endl;
    // exit(1);


    #ifdef HIPATIA
    cout << "HIPATIA is defined" << endl;
    #else
    cout << "HIPATIA is NOT defined" << endl;
    #endif




    using namespace std;
    using namespace NEAT;


#ifndef NDEBUG
    cout << "WARNING: Debug mode. Assertions enabled." << endl;
#endif

    //system("rm -r controllers*");

    if (argc < 2)
    {
        cout << "Error, no configuration file provided.\n";
        printf("double uses %d bytes in this processor/compiler.\n", sizeof(double));           // some compilers print 8
        printf("long double uses %d bytes in this processor/compiler.\n", sizeof(long double)); // some compilers print 16
        exit(0);
        // #define dim 20
        // double x_vec[dim] = {0.6757571658, 0.5135890549, 0.4608551431, 0.4998168102, 0.5278000325, 0.5246639005, 0.3951178248, 0.5214761316, 0.6219129204, 0.6766302005, 0.6855895844, 0.7734823387, 0.6537444534, 0.4777864059, 0.6669242475, 0.7482358173, 0.5883391772, 0.6494809372, 0.8319084066, 0.6674026026};
        // F3 rosembrock_test = F3(3, dim, 0.0, 10.0, 2438989325, false);
        // long double res = rosembrock_test.FitnessFunc(x_vec);


        /*
        // check optimum of langermann function (multiopt)
        #define dim 6
        double x_vec[dim] = {6.069882998, 8.680343392, 6.733060222, 8.120919558, 8.626602718, 10.45409736};
        double x_vec_copy[dim];

        for (size_t i = 0; i < dim; i++)
        {
            x_vec_copy[i] = x_vec[i];
        }
        

        long double delta = 0.01;
        long double best_res = -10000;
        cout << std::setprecision(60) << std::flush;
        F7 rosembrock_test = F7(7, dim, 0.0, 10.0, 2438989325, false);
        long double res = 0;

        // double x[3] = {1.0,1.0,1.2};
        // cout << rosembrock_test.FitnessFunc(x);

        long unsigned int i = 0;
        while(true)
        {   
            i++;
            if (i % 100000 == 0)
            {
                i = 0;
                cout << "-" << std::flush;
            }

            for (size_t s = 0; s < dim; s++)
            {
                x_vec_copy[s] = x_vec[s] + delta/(double)2*((double)random() / (double) RAND_MAX);
            }
        

            res = rosembrock_test.FitnessFunc(x_vec_copy);

            if (res > best_res || best_res==-10000)
            {
                best_res = res;
                cout << endl;
                cout << "x = ";
                PrintArray(x_vec_copy, dim);
                cout << "f(x) = " << res << endl;
            }
        }
        exit(1);
        */

    }
    else if (argc > 3)
    {
        cout << "Error, too many arguments provided."
             << "Provide only path to configuration file and optionally "
             << "use -f command to delete previous experiment.";
        exit(1);
    }

    std::string conf_file_path;
    neat_parameters* neat_params = new neat_parameters();
    INIReader reader;
    neat_params->DELETE_PREV_EXPERIMENT_FOLDER = false;


    if (std::string(argv[1]) == "-f" && argc == 2)
    {
        cout << "Error, no configuration file provided.\n";
        exit(1);
    }else if(argc == 2){
        conf_file_path = argv[1];
        reader = INIReader(argv[1]);
    }
    else if (std::string(argv[1]) == "-f" && argc == 3)
    {
        conf_file_path = argv[2];
        reader = INIReader(argv[2]);
        neat_params->DELETE_PREV_EXPERIMENT_FOLDER = true;
    }
    else if (std::string(argv[2]) == "-f" && argc == 3)
    {
        conf_file_path = argv[1];
        reader = INIReader(argv[1]);
        neat_params->DELETE_PREV_EXPERIMENT_FOLDER = true;
    }
    else
    {
        cout << "Error, more than one argument provided, and one of the arguments was not -f.";
        usage();
        exit(1);
    }

    if (reader.ParseError() != 0)
    {
        std::cout << "Can't load configuration file: " << argv[1] << "\n";
        exit(1);
    }

    std::string prob_name = reader.Get("Global", "PROBLEM_NAME", "UNKNOWN");
    if (!vector_contains_item(Experiment::get_names(), prob_name))
    {
        cout << "problem: " << prob_name << " not defined.\n";
        cout << "available experiments are: ";
        print_vector(Experiment::get_names());
        cout << "\nExiting..." << endl;
        exit(1);
    }
    else
    {
        cout << "Working on problem: " << prob_name << endl;
    }

    Experiment *exp = Experiment::get(prob_name.c_str());
    neat_params->load_global_params(conf_file_path);
    cout << "Seed: " << neat_params->SEED << endl;
    exp->neat_params = neat_params;
    global_rng.seed(neat_params->SEED);
    neat_params->SEED = global_rng.random_integer();
    global_rng.seed(neat_params->SEED);
    rng_t rng{neat_params->SEED};
    exp->run_given_conf_file(conf_file_path);
    delete neat_params;
    return 0;
}
