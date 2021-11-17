#include "std.h" // Must be included first. Precompiled header with standard library includes.
#include "std.hxx"
#include "permuevaluator.h"
#include "FitnessFunction_permu.h"
#include "map.h"
#include "network.h"
#include "networkexecutor.h"
#include "resource.h"
#include <assert.h>
#include "Population.h"
#include "Tools.h"
#include "PBP.h"
#include <cfloat>
#include "INIReader.h"
#include "loadnetworkfromfile.h"
#include <omp.h>
#include "Population.h"
#include "experiment.h"
#include "neat.h"
#include "rng.h"
#include "util.h"
#include <iomanip>
#include "Parameters.h"
#include "permuevaluator.h"
#include "PERMU_MULTI_params.h"
#include "PERMU_params.h"
#include "permu_multi_evaluator.h"
#include <vector>

using namespace std;

//#define COUNTER
//#define PRINT
//#define RANDOM_SEARCH

namespace PERMU_MULTI
{


// fitness function in sequential order
double FitnessFunction(NEAT::CpuNetwork *net, int initial_seed, int instance_index, base_params *parameters)
{
    int seed_seq = initial_seed;

    PERMU_MULTI::params_multi tmp_params = *static_cast<PERMU_MULTI::params_multi*>(parameters);
    tmp_params.INSTANCE_PATH = (*tmp_params.VECTOR_OF_INSTANCE_PATHS)[instance_index];
    double res = FitnessFunction_permu(net, seed_seq, &tmp_params);
    return res;
}



struct Evaluator
{

    PERMU_MULTI::params_multi *parameters;
    NEAT::CpuNetwork *best_network;
    int iteration_number = 0;

    __net_eval_decl Evaluator(){};




    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets)
    {
        int n_instances = (*this->parameters->VECTOR_OF_INSTANCE_PATHS).size();
        execute_multi(nets_, results, nnets, n_instances, FitnessFunction , best_network, parameters);
    }
};

} //namespace PERMU_MULTI


namespace NEAT
{

Permu_multiEvaluator::Permu_multiEvaluator()
{
    executor_multi = NEAT::NetworkExecutor<PERMU_MULTI::Evaluator>::create();
    parameters_multi = new PERMU_MULTI::params_multi();
    delete parameters;
    parameters = parameters_multi;
}

Permu_multiEvaluator::~Permu_multiEvaluator()
{
    delete executor_multi;
}

void Permu_multiEvaluator::read_conf_file(std::string conf_file_path)
{
    PermuEvaluator::read_conf_file(conf_file_path); // call parent read_config_file
    if(parameters->MODE != "train")
    {
        cerr << "Permu multiobjective with sveral instances is only available in train mode.\n"
        << "The trained controllers need to be used with the standard permu test mode." << endl;
        exit(1);
    }

    INIReader reader(conf_file_path);
    if (reader.ParseError() != 0)
    {
        std::cout << "Can't load " << conf_file_path << "\n";
        exit(1);
    }



    std::string COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS = reader.Get("Global", "COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS", "UNKNOWN");
    parameters_multi->VECTOR_OF_INSTANCE_PATHS = new std::vector<std::string>(split_string(COMMA_SEPARATED_LIST_OF_INSTANCE_PATHS, ","));
 
}

void Permu_multiEvaluator::execute(class NEAT::Network **nets_, class NEAT::OrganismEvaluation *results, size_t nnets)
{
    using namespace NEAT;
    env->pop_size = this->neat_params->POPSIZE_NEAT;
    PERMU_MULTI::Evaluator *ev = new PERMU_MULTI::Evaluator();
    this->parameters->neat_params = this->neat_params;
    ev->parameters = this->parameters_multi;
    ev->iteration_number = this->iteration_number;
    ev->best_network = this->best_network;
    ev->execute(nets_, results, nnets);

    this->best_network = ev->best_network;
    this->iteration_number++;
    delete ev;
}

void Permu_multiEvaluator::run_given_conf_file(std::string conf_file_path)
{
    using namespace std;
    using namespace NEAT;

    read_conf_file(conf_file_path);
    parameters->neat_params = this->neat_params;
    neat_params->SEED = global_rng.random_integer();

    if (parameters->MODE == "train")
    {

        Experiment *exp = Experiment::get(parameters->prob_name.c_str());
        rng_t rng{parameters->SEED};
        exp->neat_params->global_timer.tic();
        exp->run(rng);
        return;
    }
    else
    {
        cerr << "invalid mode provided. Permu multi can only be used to train. Testing is shared with permu." << endl;
        exit(1);
    }
};




class NetworkEvaluator *create_permu_multi_evaluator()
{
    return new Permu_multiEvaluator();
}

} // namespace NEAT
