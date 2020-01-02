#include "std.hxx"

#include "Parameters.h"
#include "permuevaluator.h"
#include "FitnessFunction_permu.h"
#include "map.h"
#include "network.h"
#include "networkexecutor.h"
#include "resource.h"
#include <assert.h>
#include "PBP.h"
#include "Population.h"
#include "Tools.h"
#include <cfloat>

using namespace std;

//#define COUNTER
//#define PRINT
//#define RANDOM_SEARCH



namespace NEAT
{
struct Config{};
struct Evaluator
{
    typedef NEAT::Config Config;
    const Config *config;


    __net_eval_decl Evaluator(const Config *config_) : config(config_){};


    // fitness function in sequential order
    __net_eval_decl double FitnessFunction(CpuNetwork *net, int n_evals, int initial_seed)
    {
        int seed_seq = initial_seed;
        double res = FitnessFunction_permu(net, n_evals, seed_seq);
        seed_seq += n_evals;
        return res;
    }

    // parallelize over the same network
    __net_eval_decl void FitnessFunction_parallel(CpuNetwork *net, int n_evals, double *res, int initial_seed)
    {
        int seed_parallel = initial_seed;

        #pragma omp parallel for num_threads(N_OF_THREADS)
        for (int i = 0; i < n_evals; i++)
        {
            res[i] = FitnessFunction_permu(net, N_EVALS, seed_parallel + i);
        }
        seed_parallel += n_evals;
    }

    // Fitness all train
    __net_eval_decl void FitnessAllTrain(class Network **nets_, OrganismEvaluation *results, size_t nnets){
        CpuNetwork **nets = (CpuNetwork **)nets_;
        double progress_print_decider = 0.0;
        double *f_values = new double[nnets];
        bool printed_bracket = false;
        RandomNumberGenerator rng;
        rng.seed();
        int initial_seed = rng.random_integer_fast(10050000,20000000);
        // evaluate the individuals
        #pragma omp parallel for num_threads(N_OF_THREADS)
        for (size_t inet = 0; inet < nnets; inet++)
        {
            CpuNetwork *net = nets[inet];
            Evaluator *ev = new Evaluator(config);
            OrganismEvaluation eval;
            int seed = initial_seed;
            f_values[inet] = ev->FitnessFunction(net, N_EVALS, seed);
            results[inet] = eval;
            delete ev;

            // print progress.
            std::mutex mutx;
            mutx.lock();
            if (!printed_bracket)
            {
                std::cout << "[" << std::flush;
                printed_bracket = true;
            }
            progress_print_decider += 15.0 / (double)nnets;
            if (inet == 0)
            {
            }
            while (progress_print_decider >= 1.0)
            {
                std::cout << "." << std::flush;
                progress_print_decider--;
            }
            mutx.unlock();
        }
        std::cout << "]" << std::endl;

        // // reevaluate top n_of_threads_omp, with a minimum of 5 and a maximum of nnets.
        // double cut_value = obtain_kth_largest_value(f_values, min(max(n_of_threads_omp, 5), static_cast<int>(nnets)), nnets);

        // reevaluate top 5% at least N_REEVAL times
        int actual_n_reevals = (((N_REEVALS_TOP_5_PERCENT - 1) / N_OF_THREADS) + 1) * N_OF_THREADS;
        int n_of_networks_to_reevaluate = max(1, static_cast<int>(nnets) * 5 / 100);
        cout << "reevaluating top 5% (" << n_of_networks_to_reevaluate << " nets out of " << static_cast<int>(nnets) << ") each " << actual_n_reevals << " times." << endl;

        double cut_value = obtain_kth_largest_value(f_values, n_of_networks_to_reevaluate, static_cast<int>(nnets));

        rng.seed();
        initial_seed = rng.random_integer_fast(20050000,30000000);

        for (size_t inet = 0; inet < nnets; inet++)
        {
            if (f_values[inet] <= cut_value)
            {
                f_values[inet] -= 1000000000.0; // apply a discount to the individuals that are not reevaluated
                continue;
            }
            else
            {
                CpuNetwork *net = nets[inet];
                Evaluator *ev = new Evaluator(config);
                double *res = new double[actual_n_reevals];
                int seed = initial_seed;
                ev->FitnessFunction_parallel(net, actual_n_reevals, res, seed);
                int index_value = arg_element_in_centile_specified_by_percentage(res, actual_n_reevals, 0.50);
                f_values[inet] = res[index_value];
                delete[] res;
                delete ev;
            }
        }

        cout << "Reevaluating best indiv of generation: ";
        int index_most_fit = argmax(f_values, nnets);
        CpuNetwork *net = nets[index_most_fit];
        Evaluator *ev = new Evaluator(config);

        // apply a discount to all but the best individual
        for (int i = 0; i < (int) nnets; i++)
        {
            if (i != index_most_fit)
            {
                f_values[i] -= 1000000000.0; 
            }
        }

        double *res = new double[N_EVALS_TO_UPDATE_BK];
        ev->FitnessFunction_parallel(net, N_EVALS_TO_UPDATE_BK, res, 30050000);


        // double median = res[arg_element_in_centile_specified_by_percentage(res, N_EVALS_TO_UPDATE_BK, 0.5)];
        double average = Average(res, N_EVALS_TO_UPDATE_BK);


        cout << "best this gen: " << average << endl;

        if (average > BEST_FITNESS_TRAIN)        {


            bool update_needed = is_A_larger_than_B_Mann_Whitney(res, F_VALUES_OBTAINED_BY_BEST_INDIV, N_EVALS_TO_UPDATE_BK);

            if (update_needed)
            {
                N_TIMES_BEST_FITNESS_IMPROVED_TRAIN++;
                cout << "[BEST_FITNESS_IMPROVED] --> " << average << endl;
                BEST_FITNESS_TRAIN = average;
                copy_vector(F_VALUES_OBTAINED_BY_BEST_INDIV, res, N_EVALS_TO_UPDATE_BK);
            }
        }

        delete ev;
        delete[] res;

        double* tmp_order = new double[nnets];

        cout << "fitness_array: " << std::flush;
        //PrintArray(f_values, nnets);

        compute_order_from_double_to_double(f_values, nnets, tmp_order, false, true);

        std::swap(f_values, tmp_order);



        multiply_array_with_value(f_values, 1.0 / (double) (nnets-1), nnets);
        multiply_array_with_value(f_values, 1.0 + ((double)N_TIMES_BEST_FITNESS_IMPROVED_TRAIN / 1000.0), nnets);

        cout << "fitness_array: " << std::flush;
        //PrintArray(f_values, nnets);

        // save scaled fitness
        for (size_t inet = 0; inet < nnets; inet++)
        {
            results[inet].fitness = f_values[inet];
            results[inet].error = 2 - f_values[inet];
        }
        delete[] tmp_order;
        delete[] f_values;
    
    }
};

class PermuEvaluator : public NetworkEvaluator
{
    NetworkExecutor<Evaluator> *executor;

public:
    PermuEvaluator()
    {
        executor = NetworkExecutor<Evaluator>::create();
    }

    ~PermuEvaluator()
    {
        delete executor;
    }

    virtual void execute(class Network **nets_,
                         class OrganismEvaluation *results,
                         size_t nnets)
    {
        NEAT::Config* config;
        Evaluator *ev = new Evaluator(config);
        ev->FitnessAllTrain(nets_, results, nnets);
    }
};

class NetworkEvaluator *create_permu_evaluator()
{
    return new PermuEvaluator();
}

} // namespace NEAT
