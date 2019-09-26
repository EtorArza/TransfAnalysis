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
#include "permuevaluator.h"
#include "Tools.h"
#include "FitnessFunction_permus.h"


#define EXTERN
#include "Parameters.h"
#include <iomanip>      // std::setprecision


using namespace NEAT;
using namespace std;

#define DEFAULT_RNG_SEED 1
#define DEFAULT_MAX_TRAIN_TIME 2*24*60*60
#define DEFAULT_PARALLEL_THREADS 1

void usage()
{

    // int *permu = new int[12];
    // int *temp = new int[12];

    // GenerateRandomPermutation(permu, 12);

    // permu[0] = 0;

    // PrintArray(permu, 12);
    // compute_order(permu, 12, temp);
    // // cout << obtain_kth_largest_value(permu, 1, 12) << endl;
    // // cout << obtain_kth_smallest_value(permu, 3, 12) << endl;
    // PrintArray(temp, 12);



    cerr << "usage: ./neat train [OPTIONS]" << endl;
    cerr << " OR " << endl;
    cerr << "usage: ./neat test controller_path instance_path problem_type" << endl;



    cerr << "--- TRAIN ---" << endl;

    cerr << "[OPTIONS] for train" << endl;
    cerr << "  -f                   Force deletion of any data from previous run." << endl;
    cerr << "  -r RNG_seed          (default=" << DEFAULT_RNG_SEED << ")" << endl;
    cerr << "  -n population_size   (default=" << env->pop_size << ")" << endl;
    cerr << "  -x max_train_time   (default=" << DEFAULT_MAX_TRAIN_TIME << ")" << endl;
    cerr << "  -s search_type       {phased, blended, complexify} (default=phased)" << endl;
    cerr << "  -t num_of_parallel_threads (default=" << DEFAULT_PARALLEL_THREADS << ")" << endl;



    cerr << endl;
    cerr << endl;
    cerr << endl;

    cerr << "--- TEST ---" << endl;
    cerr << "controller_path -> path to the file containing the controller." << endl;
    cerr << "instances_path -> path to the file containing problem instance." << endl;
    cerr << "problem_type -> Either \"tsp\", \"lop\", \"qap\" or \"PFSP\" " << endl;
    cerr << "for example: \n\n./neat \"test\" \"experiment_1/fittest_1500\" \"instances\\lop\\instance.txt\" \"lop\"" << endl;

    cerr << endl;
    // cerr << "experiment names: ";
    // auto names = Experiment::get_names();
    // for(size_t i = 0; i < names.size(); i++) {
    //     if(i != 0)
    //         cerr << ", ";
    //     cerr << names[i];
    // }
    // cerr << endl;
    cerr << endl;

    exit(1);
}





int main(int argc, char *argv[])
{

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


        bool force_delete = reader.GetBoolean("NEAT","DELETE_PREVIOUS_EXPERIMENT", false);
        int rng_seed = reader.GetInteger("NEAT","SEED", -1);
        env->pop_size = reader.GetInteger("NEAT", "POPSIZE", -1);
        int max_time = reader.GetInteger("NEAT", "MAX_TRAIN_TIME", -1);
        n_of_threads_omp = reader.GetInteger("NEAT", "THREADS", -1);
        N_EVALS = reader.GetInteger("NEAT", "N_EVALS", -1);
        N_REEVALS = reader.GetInteger("NEAT","N_REEVALS", -1);
        string search_type = reader.Get("NEAT", "SEARCH_TYPE", "UNKOWN");
        PROBLEM_TYPE = reader.Get("Controller", "PROBLEM_TYPE", "UNKOWN");
        INSTANCE_PATH = reader.Get("Controller", "PROBLEM_PATH", "UNKOWN");
        MAX_TIME_PSO = reader.GetReal("Controller", "MAX_TIME_PSO", -1.0);
        POPSIZE = reader.GetInteger("Controller", "POPSIZE", -1);
        TABU_LENGTH = reader.GetInteger("Controller", "TABU_LENGTH", -1);



        if (N_OF_INPUT_PARAMS_TEST == argc)
        {
            MAX_TIME_PSO = stof(argv[4]);
        }

        if (search_type == "complexify")
        {
            env->search_type = GeneticSearchType::PHASED;
        }else if (search_type == "blended"){
            env->search_type = GeneticSearchType::BLENDED;
        }else if (search_type == "complexify"){
            env->search_type = GeneticSearchType::COMPLEXIFY;
        }else{
            cout << "Error, no search type specified." << endl;
        }


        if(n_of_threads_omp < 0){
            cout << "please specify a valid number of threads on the conf. file" <<endl;
            exit(1);
        }


        if (force_delete)
        {
            sh("rm -rf experiment_*");
        }
        else if (exists("experiment_1"))
        {
            error("Already exists: experiment_1.\nMove your experiment directories or use -f to delete them automatically. If -f is used, all previous experiments will be deleted.")
        }

        omp_set_num_threads(n_of_threads_omp);

        if (n_of_threads_omp < 7)
        {
            cout << "Warning: a minimum of 7 threads is recommended for this implementation of NEAT to function correctly." << endl;
            cout << "With the current settings, processing a generation takes around " ; 
            cout << ((env->pop_size*MAX_TIME_PSO / (double)2 * (double)N_EVALS + (double)(5/n_of_threads_omp + 1) * (double)N_REEVALS)) / 3600.0; 
            cout << "h,  which might be too long." << endl << endl;
        }

        if (env->pop_size < 1000)
        {
            cout << "Warning: The population size of the controllers might be too low." << endl;
            cout << "The provided population size of the controllers is " << env->pop_size << ", a value of at least 1000 is recommended." << endl;
            cout << "For example, to use a population size of 1500, use the \"-n 1500\" argument." << endl;
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
        exp->run(rng, 10000000);
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
        CONTROLLER_PATH = reader.Get("Controller", "CONTROLLER_PATH", "UNKNOWN");

        if (CONTROLLER_PATH == "UNKNOWN")
        {
            cout << "error, controller path not specified in test." << endl;
        }
        
        CpuNetwork net = load_network(CONTROLLER_PATH);
        double f_best = FitnessFunction_permu(&net, 1);

        //cout << INSTANCE_PATH << "|" << PROBLEM_TYPE << "|" << f_best << endl;
        cout << std::setprecision(15);
        cout << std::flush;
        cout << f_best << std::endl;;
        cout << std::flush;

        return 0;
    }
    else
    {
        cout << "invalid mode provided. Please, use the configuration file to specify either test or train.";
        exit(1);
    }
}
