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
#include "../permus/permu_problem_src/FitnessFunction_permu.h"

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


    void get_net_ranking(NEAT::CpuNetwork **nets, size_t nnets, int n_evals, PERMU::params *parameters, int* ranking){
        RandomNumberGenerator *rng = new RandomNumberGenerator();
        rng->seed();
        int SEED = rng->random_integer_fast((int) 10e7);
        double av_fitnesses[nnets];

        #pragma omp parallel for num_threads(N_OF_THREADS)
        for (size_t i = 0; i < nnets; i++)
        {
            av_fitnesses[i] = FitnessFunction_permu(nets[i], n_evals, SEED, parameters);
        }
        compute_order_from_double_to_int(av_fitnesses, (int) nnets, ranking, true);
    }



    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets)
    {
        int approx_ranking_samples = 100;
        int true_ranking_samples = 5000;
        vector<int> samplesizes{ 1, 2, 5, 10, 20, 50};

        NEAT::CpuNetwork **nets = (NEAT::CpuNetwork **)nets_;
        int ranking[nnets];
        int true_ranking[nnets];
        double avg_distances[99];
        set_array_to_value(avg_distances, 0.0, 99);

        get_net_ranking(nets, nnets, true_ranking_samples, parameters, true_ranking);

        PERMU::PBP *problem;
        problem = PERMU::GetProblemInfo(parameters->PROBLEM_TYPE, parameters->INSTANCE_PATH);     //Read the problem instance to optimize.
        int instance_size = problem->GetProblemSize();
        delete problem;

        cout << "Kendall_dist: ";
        for (size_t i = 0; i < samplesizes.size(); i++)
        {
            get_net_ranking(nets, nnets, samplesizes[i], parameters, ranking);
            for (size_t k = 0; k < approx_ranking_samples; k++)
            {   
                cout << "(i,k,d) = (" << i << "," << k << "," << Kendall(ranking, true_ranking, nnets) << ")";
                avg_distances[i] += (double) Kendall(ranking, true_ranking, nnets);
            }
            avg_distances[i] /= (double) (approx_ranking_samples * n_choose_k(instance_size,2));
        }
        std::string res_file_path = "true_ranking_res.csv";
        append_line_to_file(res_file_path, "sample size , distance to the true ranking\n");

        for (size_t i = 0; i < samplesizes.size(); i++)
        {
            std::string new_line_to_append = std::to_string(samplesizes[i]);
            new_line_to_append += ",";
            new_line_to_append += std::to_string(avg_distances[i]);
            new_line_to_append += "\n";
            append_line_to_file(res_file_path, new_line_to_append);

        }
        
        PrintArray(avg_distances, samplesizes.size());
        exit(0);
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
