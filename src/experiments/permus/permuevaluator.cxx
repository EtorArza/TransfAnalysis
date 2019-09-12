#include "std.hxx"

#include "Parameters.h"
#include "permuevaluator.h"
#include "FitnessFunction_permus.h"
#include "map.h"
#include "network.h"
#include "networkexecutor.h"
#include "resource.h"
#include <assert.h>
#include "PBP.h"
//#include "PFSP.h"
#include "LOP.h"
#include "QAP.h"
//#include "TSP.h"
//#include "API.h"
#include "Population.h"
#include "Tools.h"


using namespace std;

float FitnessFunction_permu(NEAT::CpuNetwork *net)
{
    float* v_of_fitness;
    PBP *problem;
    CPopulation *pop;
    //char* params[3] = {"binary_name", "lop", "src/experiments/permus/instances/lop/Cebe.lop.n30.1"};
    char const *params[3] = {"binary_name", "qap", "src/experiments/permus/instances/qap/tai35a.dat.dat"};
    set_parameters(3, params); // Read parameters from bash.
    //Read the problem instance to optimize.
    problem = GetProblemInfo(PROBLEM_TYPE, INSTANCE_PATH);
    pop = new CPopulation(problem);
    v_of_fitness = new float[REPEATED_EVALUATIONS];


    for (int n_of_repetitions_completed = 0; n_of_repetitions_completed < REPEATED_EVALUATIONS; n_of_repetitions_completed++)
    {
        pop->rng->seed();
        pop->Reset();
        while (!pop->terminated)
        {
            for (int i = 0; i < POPSIZE; i++)
            {
                net->clear_noninput();
                for (int sns_idx = 0; sns_idx < NEAT::__sensor_N; sns_idx++)
                {
                    net->load_sensor(sns_idx, pop->get_neat_input_individual_i(i)[sns_idx]);
                }
                net->activate();
                pop->apply_neat_output_to_individual_i(net->get_outputs(), i);
            }
            pop->end_iteration();
        }
        v_of_fitness[n_of_repetitions_completed] = pop->f_best;
    }
    float res = Average(v_of_fitness, REPEATED_EVALUATIONS);
    delete[] v_of_fitness;
    delete pop;
    delete problem;
    return res;

}

namespace NEAT
{
struct Config{};
struct Evaluator
{
    typedef NEAT::Config Config;
    const Config *config;
    __net_eval_decl Evaluator(const Config *config_): config(config_){};
    // Check if evaluation is terminated. If it is, __net_eval_decl OrganismEvaluation result() is called.
    __net_eval_decl bool next_step()
    {
        return true;
    }
    __net_eval_decl float FitnessFunction(CpuNetwork* net){
        return FitnessFunction_permu(net);
    }
};

class PermuEvaluator : public NetworkEvaluator
{
    NetworkExecutor<Evaluator> *executor;

public:
    PermuEvaluator()
    {
        executor = NetworkExecutor<Evaluator>::create();
        Evaluator::Config *config;


        // size_t configlen;
        // create_config(config, configlen);
        // executor->configure(config, configlen);
        // free(config);
    }

    ~PermuEvaluator()
    {
        delete executor;
    }

    virtual void execute(class Network **nets_,
                         class OrganismEvaluation *results,
                         size_t nnets)
    {
        executor->execute(nets_, results, nnets);
    }




};

class NetworkEvaluator *create_permu_evaluator()
{
    return new PermuEvaluator();
}

} // namespace NEAT
