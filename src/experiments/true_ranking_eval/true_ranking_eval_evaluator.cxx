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
#include <random> // std::default_random_engine

using namespace std;


#define SAMPLES_TRUE_RANKING 4000
#define SAMPLES_KENDALL 20
#define SAMPLES_SELECT_BEST 20
#define TARGET_PERC_KENDALL 0.05
#define TARGET_PERCENTAGE_SELECT_BEST 0.7999
#define STEP_EVALS 5

//#define COUNTER
//#define PRINT
//#define RANDOM_SEARCH

namespace true_ranking_eval
{

struct Evaluator
{

    PERMU::params *parameters;
    int true_ranking_samples;
    double **f_values;

    __net_eval_decl Evaluator(){};

    // fitness function in sequential order
    __net_eval_decl double FitnessFunction(NEAT::CpuNetwork *net, int n_evals, int initial_seed, int instance_index){};
    // parallelize over the same network
    __net_eval_decl void FitnessFunction_parallel(NEAT::CpuNetwork *net, int n_evals, double *res, int initial_seed, int instance_index){};

    int get_best_net_index(NEAT::CpuNetwork **nets, size_t nnets, int n_evals, PERMU::params *parameters)
    {
        int *ranking = new int[nnets];
        get_net_ranking(nets, nnets, n_evals, parameters, ranking);
        int res = index(ranking, 0, nnets);
        delete[] ranking;
        return res;
    }

    void get_net_ranking(NEAT::CpuNetwork **nets, size_t nnets, int n_evals, PERMU::params *parameters, int *ranking)
    {
        RandomNumberGenerator *rng = new RandomNumberGenerator();
        rng->seed();
        int SEED = rng->random_integer_fast();
        double av_fitnesses[nnets];

        #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS)
        for (int i = 0; i < nnets; i++)
        {
            av_fitnesses[i] = FitnessFunction_permu(nets[i], n_evals, SEED + i * n_evals, parameters);
        }
        compute_order_from_double_to_int(av_fitnesses, (int)nnets, ranking, true);
        delete rng;
    }

    void reduce_size_true_ranking(const int *true_ranking, int small_popsize, int *small_true_ranking)
    {
        int tmp_ranking[small_popsize];
        for (int i = 0; i < small_popsize; i++)
        {
            tmp_ranking[i] = i;
        }

        sort(tmp_ranking, tmp_ranking + small_popsize, [true_ranking, small_popsize](int a, int b) { return true_ranking[a] < true_ranking[b]; });
        Invert(tmp_ranking, small_popsize, small_true_ranking);
    }

    void experiment_on_probability_of_selecting_best(NEAT::CpuNetwork **nets, size_t nnets, int* true_ranking, vector<int> popsizes)
    {
        int samples_to_estimate_prob_of_selecting_best = SAMPLES_SELECT_BEST;
        double target_prob_of_selecting_best = TARGET_PERCENTAGE_SELECT_BEST;
        int best_controller_index;
        int *small_true_ranking = new int[nnets];
        std::string res_file_path = "samples_to_estimate_prob_of_selecting_best.csv";
        append_line_to_file(res_file_path, "popsize, sample size, prob. selecting top 3\n");
        for (int i = 0; i < popsizes.size(); i++)
        {   
            reduce_size_true_ranking(true_ranking, popsizes[i], small_true_ranking);
            double prob = 0.0;
            int n_evals = 0;
            while (prob < target_prob_of_selecting_best)
            {
                n_evals += STEP_EVALS;
                prob = 0.0;
                int estimated_best_controller_index;
                for (int j = 0; j < samples_to_estimate_prob_of_selecting_best; j++)
                {
                    estimated_best_controller_index = get_best_net_index(nets, popsizes[i], n_evals, parameters);
                    if (index(small_true_ranking, 0, popsizes[i]) == estimated_best_controller_index ||
                        index(small_true_ranking, 1, popsizes[i]) == estimated_best_controller_index ||
                        index(small_true_ranking, 2, popsizes[i]) == estimated_best_controller_index
                    )
                    {
                        prob += 1.0 / (double)samples_to_estimate_prob_of_selecting_best;
                    }
                }
                cout << "(prob, n_evals, popsizes[i], best_controller_index, estimated_best_controller_index) -> " << prob << ", " << n_evals << ", " << popsizes[i] << ", " << best_controller_index << ", " << estimated_best_controller_index << endl;
            }



        std::string new_line_to_append = std::to_string(popsizes[i]);
        new_line_to_append += ",";
        new_line_to_append += std::to_string(n_evals);
        new_line_to_append += ",";
        new_line_to_append += std::to_string(prob);
        new_line_to_append += "\n";
        append_line_to_file(res_file_path, new_line_to_append);
        }
        delete[] small_true_ranking;
    }

    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets)
    {
        NEAT::CpuNetwork **nets = (NEAT::CpuNetwork **)nets_;
        int ranking[nnets];
        int true_ranking[nnets];
        int small_true_ranking[nnets];
        double avg_distances[99];
        int required_evals[99];
        set_array_to_value(avg_distances, 0.0, 99);
        set_array_to_value(required_evals, 0, 99);

        int approx_ranking_samples = SAMPLES_KENDALL;
        true_ranking_samples = SAMPLES_TRUE_RANKING;
        double desired_percentage_threshold = TARGET_PERC_KENDALL;
        vector<int> popsizes{128, 256, 513, 1024};

        get_net_ranking(nets, nnets, true_ranking_samples, parameters, true_ranking);


        experiment_on_probability_of_selecting_best(nets, nnets, true_ranking, popsizes);

        PERMU::PBP *problem;
        problem = PERMU::GetProblemInfo(parameters->PROBLEM_TYPE, parameters->INSTANCE_PATH); //Read the problem instance to optimize.
        int instance_size = problem->GetProblemSize();
        delete problem;

        cout << "Kendall_dist: \n";
        for (size_t i = 0; i < popsizes.size(); i++)
        {
            int current_samplesize = 0;
            double percentage_kendal = 1.0;
            reduce_size_true_ranking(true_ranking, popsizes[i], small_true_ranking);

            while (percentage_kendal > desired_percentage_threshold)
            {
                current_samplesize += STEP_EVALS;

                if (current_samplesize > 200)
                {
                    exit(1);
                }
                avg_distances[i] = 0.0;
                percentage_kendal = 0.0;
                for (size_t k = 0; k < approx_ranking_samples; k++)
                {
                    get_net_ranking(nets, popsizes[i], current_samplesize, parameters, ranking);
                    int d = (double)Kendall(ranking, small_true_ranking, popsizes[i]);
                    avg_distances[i] += d;
                    // PrintArray(ranking, popsizes[i]);
                    // PrintArray(true_ranking, popsizes[i]);
                    // PrintArray(small_true_ranking, popsizes[i]);

                    // cout << "---" << endl;
                }
                avg_distances[i] /= (double)(approx_ranking_samples);
                avg_distances[i] /=  (double)n_choose_k(popsizes[i], 2);
                percentage_kendal = avg_distances[i];
                cout << popsizes[i] << ", " << current_samplesize << ", " << percentage_kendal << endl;
            }
            required_evals[i] = current_samplesize;
        }

        std::string res_file_path = "true_ranking_res.csv";
        append_line_to_file(res_file_path, "popsize, sample size , percentage of wrong rankings\n");

        for (size_t i = 0; i < popsizes.size(); i++)
        {
            std::string new_line_to_append = std::to_string(popsizes[i]);
            new_line_to_append += ",";
            new_line_to_append += std::to_string(required_evals[i]);
            new_line_to_append += ",";
            new_line_to_append += std::to_string(avg_distances[i]);
            new_line_to_append += "\n";
            append_line_to_file(res_file_path, new_line_to_append);
        }

        PrintArray(avg_distances, popsizes.size());
        exit(0);
    }
};

} //namespace true_ranking_eval

namespace NEAT
{

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
        if (this->neat_params->IS_LAST_ITERATION)
        {
            true_ranking_eval::Evaluator *ev = new true_ranking_eval::Evaluator();
            NEAT::env->pop_size = neat_params->POPSIZE_NEAT;
            this->parameters->neat_params = this->neat_params;
            ev->parameters = this->parameters;
            ev->execute(nets_, results, nnets);
            delete ev;
        }
        else
        {
            PermuEvaluator::execute(nets_, results, nnets);
        }
    }

    virtual void run_given_conf_file(std::string conf_file_path) override
    {
        using namespace std;
        using namespace NEAT;

        read_conf_file(conf_file_path);

        Experiment *exp = Experiment::get(parameters->prob_name.c_str());
        rng_t rng{parameters->SEED};
        this->neat_params->global_timer.tic();
        exp->neat_params = this->neat_params;
        exp->run(rng);
    }
};

class NetworkEvaluator *create_true_ranking_eval_evaluator()
{
    return new true_ranking_evalEvaluator();
}

} // namespace NEAT
