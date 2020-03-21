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
#include "true_ranking_eval_params.h"
#include "../permus/permuevaluator.h"
#include "../permus/PERMU_params.h"


using namespace std;

//#define COUNTER
//#define PRINT
//#define RANDOM_SEARCH

namespace true_ranking_eval
{

struct Evaluator
{

    PERMU::params *parameters;


    __net_eval_decl Evaluator(){};

    // fitness function in sequential order
    __net_eval_decl double FitnessFunction(NEAT::CpuNetwork *net, int n_evals, int initial_seed, int instance_index){    };
    // parallelize over the same network
    __net_eval_decl void FitnessFunction_parallel(NEAT::CpuNetwork *net, int n_evals, double *res, int initial_seed, int instance_index){    };

    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets)
    {
        NEAT::CpuNetwork **nets = (NEAT::CpuNetwork **)nets_;
        cout << "you got those nets" << endl;

    }
};

} //namespace true_ranking_eval

namespace NEAT{

class true_ranking_evalEvaluator : public PermuEvaluator
{
    NetworkExecutor<true_ranking_eval::Evaluator> *executor;

public:
    true_ranking_evalEvaluator()
    {
        executor = NEAT::NetworkExecutor<true_ranking_eval::Evaluator>::create();
        parameters = new PERMU::params();
    }

    ~true_ranking_evalEvaluator()
    {
        delete executor;
    }

    virtual void execute(class NEAT::Network **nets_, class NEAT::OrganismEvaluation *results, size_t nnets) override
    {
        NEAT::env->pop_size = POPSIZE_NEAT;
        true_ranking_eval::Evaluator *ev = new true_ranking_eval::Evaluator();
        ev->parameters = this->parameters;
        ev->execute(nets_, results, nnets);
        delete ev;
    }

    virtual void run_given_conf_file(std::string conf_file_path) override
    {
        using namespace std;
        using namespace NEAT;

        read_conf_file(conf_file_path);

        Experiment *exp = Experiment::get(parameters->prob_name.c_str());
        rng_t rng{parameters->SEED};
        global_timer.tic();
        exp->run(rng);
    }


};



class NetworkEvaluator *create_true_ranking_eval_evaluator()
{
    return new true_ranking_evalEvaluator();
}

} // namespace NEAT
