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

//#define COUNTER


double FitnessFunction_permu( NEAT::CpuNetwork *net, int n_evals)
{

    double* v_of_fitness;
    PBP *problem;
    CPopulation *pop;

    //Read the problem instance to optimize.
    problem = GetProblemInfo(PROBLEM_TYPE, INSTANCE_PATH);
    pop = new CPopulation(problem);
    v_of_fitness = new double[n_evals];


    for (int i = 0; i < POPSIZE; i++)
    {
        pop->m_individuals[i]->activation = std::vector<double>(net->activations);
    }


    #ifdef COUNTER
    int counter = 0;
    #endif

    for (int n_of_repetitions_completed = 0; n_of_repetitions_completed < n_evals; n_of_repetitions_completed++)
    {
        pop->rng->seed();
        pop->Reset(); // here all networks are deleted denwedfjhfrjoirefojirewerioj
        for (int i = 0; i < POPSIZE; i++)
        {   
            std::swap(net->activations, pop->m_individuals[i]->activation);
            net->clear_noninput(); 
            std::swap(net->activations, pop->m_individuals[i]->activation);
        }
        while (!pop->terminated)
        {   
            #ifdef COUNTER
            counter ++;
            #endif

            for (int i = 0; i < POPSIZE; i++)
            {   
                
                std::swap(net->activations, pop->m_individuals[i]->activation);
                for (int sns_idx = 0; sns_idx < NEAT::__sensor_N; sns_idx++)
                {
                    net->load_sensor(sns_idx, pop->get_neat_input_individual_i(i)[sns_idx]);
                }
                net->activate();
                pop->apply_neat_output_to_individual_i(net->get_outputs(), i);
                std::swap(net->activations, pop->m_individuals[i]->activation);
            }
            pop->end_iteration();
        }
        if (!isPermutation(pop->genome_best, pop->n))
        {
            cout << "final result is not permutation" << endl;
            cout << "final permu: " ;
            PrintArray(pop->genome_best, pop->n);
            exit(1);
        }
        
        v_of_fitness[n_of_repetitions_completed] = problem->Evaluate(pop->genome_best);
    }
    double res = Average_drop_top_bottom_quartile(v_of_fitness, n_evals);
    #ifdef COUNTER
    cout << counter << endl;
    #endif

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



    __net_eval_decl double FitnessFunction(CpuNetwork* net){
        double res = FitnessFunction_permu(net, N_EVALS);
        return res;
    }

    __net_eval_decl double FitnessFunction_reevaluation(CpuNetwork* net){
        double res = FitnessFunction_permu(net, N_REEVALS);
        return res;
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
