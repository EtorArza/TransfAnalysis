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

#define EXTERN
#include "Parameters.h"


using namespace NEAT;
using namespace std;

#define DEFAULT_RNG_SEED 1
#define DEFAULT_MAX_GENS 10000
#define DEFAULT_PARALLEL_THREADS 1

void usage()
{
    cerr << "usage: ./neat train [OPTIONS]" << endl;
    cerr << " OR " << endl;
    cerr << "usage: ./neat test controller_path instance_path problem_type" << endl;



    cerr << "--- TRAIN ---" << endl;

    cerr << "[OPTIONS] for train" << endl;
    cerr << "  -f                   Force deletion of any data from previous run." << endl;
    cerr << "  -c num_experiments   (default=" << env->num_runs << ")" << endl;
    cerr << "  -r RNG_seed          (default=" << DEFAULT_RNG_SEED << ")" << endl;
    cerr << "  -n population_size   (default=" << env->pop_size << ")" << endl;
    cerr << "  -x max_generations   (default=" << DEFAULT_MAX_GENS << ")" << endl;
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

template <typename T>
T parse_enum(const char *opt, string str, map<string, T> vals)
{
    auto it = vals.find(str);
    if (it == vals.end())
    {
        error("Invalid value for " << opt << ": " << str);
    }
    return it->second;
}

int parse_int(const char *opt, const char *str)
{
    try
    {
        return stoi(str);
    }
    catch (...)
    {
        error("Expecting integer argument for " << opt << ", found '" << str << "'.");
    }
}


PBP *GetProblemInfo(std::string problemType, std::string filename)
{
    PBP *problem;
    // if (problemType == "pfsp")
    //     problem = new PFSP();
    // else if (problemType == "tsp")
    //     problem = new TSP();else 
    if (problemType == "qap")
        {problem = new QAP();}
    else if (problemType == "lop")
        {problem = new LOP();}
    // else if (problemType == "api")
    //     problem = new API();
    else
    {
         cout << "Wrong problem type was specified." << endl;
         exit(1);
     }

    //Read the instance.
    problem->Read(filename);
    return problem;
}


int main(int argc, char *argv[])
{
    int rng_seed = DEFAULT_RNG_SEED;
    int maxgens = DEFAULT_MAX_GENS;
    int threads = DEFAULT_PARALLEL_THREADS;
    bool force_delete = false;

    if (argc < 3)
    {
        usage();
        exit(1);
    }
    else if (strcmp(argv[1], "train") == 0)
    {

        {
            int opt;
            while ((opt = getopt(argc, argv, "fc:r:p:g:n:x:t:s:")) != -1)
            {
                switch (opt)
                {
                case 'f':
                    force_delete = true;
                    break;
                case 'c':
                    env->num_runs = parse_int("-c", optarg);
                    break;
                case 'r':
                    rng_seed = parse_int("-r", optarg);
                    break;
                case 'n':
                    env->pop_size = parse_int("-n", optarg);
                    break;
                case 'x':
                    maxgens = parse_int("-x", optarg);
                    break;
                case 't':
                    threads = parse_int("-t", optarg);
                    break;
                case 's':
                    env->search_type = parse_enum<GeneticSearchType>("-s", optarg, {{"phased", GeneticSearchType::PHASED}, {"blended", GeneticSearchType::BLENDED}, {"complexify", GeneticSearchType::COMPLEXIFY}});
                    break;
                default:
                    error("Invalid option: -" << (char)opt);
                }
            }
        }

        int nargs = argc - optind;
        if (nargs == 0)
        {
            usage();
        }
        else if (nargs > 1)
        {
            error("Unexpected argument: " << argv[optind + 1]);
        }

        if (force_delete)
        {
            sh("rm -rf experiment_*");
        }
        else if (exists("experiment_1"))
        {
            error("Already exists: experiment_1.\nMove your experiment directories or use -f to delete them automatically. If -f is used, all previous experiments will be deleted.")
        }

        omp_set_num_threads(threads);

        if (env->search_type == GeneticSearchType::BLENDED)
        {
            env->mutate_delete_node_prob *= 0.1;
            env->mutate_delete_link_prob *= 0.1;
        }

        const char * prob_name = "permu";
        Experiment *exp = Experiment::get(prob_name);

        rng_t rng{rng_seed};
        exp->run(rng, maxgens);
        return 0;

    }else if (strcmp(argv[1], "test") == 0){


    //const char * prob_name = "permu";
    //Experiment *exp = Experiment::get(prob_name);




    set_parameters(argc - 1, argv+ 1);

    CpuNetwork net = load_network(CONTROLLER_PATH);
    // CpuNetwork* net = (CpuNetwork* ) &org->net;
    


    PBP* problem;
    CPopulation* pop;

   float *controller_input = new float[NEAT::__sensor_N];
   float *controller_output = new float[NEAT::__output_N];


    //Read the problem instance to optimize.
    problem = GetProblemInfo(PROBLEM_TYPE, INSTANCE_PATH);
    pop = new CPopulation(problem);

    
    int counter = 0;
    while (!pop->terminated)
    {   
        if (counter % 100 == 0)
        {
        cout << pop->f_best << endl;
        }
        

        for (int i = 0; i < POPSIZE; i++)
        {
            controller_input = pop->get_neat_input_individual_i(i); // compute input
            net.clear_noninput();
            for (size_t j = 0;j < NEAT::__sensor_N; j++) // load inputs
            {   
                net.load_sensor(controller_input[j], j);
            }
            net.activate();
            controller_output = net.get_outputs();
            pop->apply_neat_output_to_individual_i(controller_output, i);
        }
        pop->end_iteration();
        counter++;
    }

    if (pop->f_best != problem->Evaluate(pop->genome_best))
    {
        cout << "Error, the fitness of the best individual in the population does not match the best fitness." << endl;
        exit(1);
    }


    cout << INSTANCE_PATH << "|" << PROBLEM_TYPE << "|" << pop->f_best << endl;


    return 0;
    }else{
        usage();
        exit(1);
    }

}
