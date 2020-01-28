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
#include "PERMU_MULTI_params.h"

using namespace std;

//#define COUNTER
//#define PRINT
//#define RANDOM_SEARCH

namespace PERMU_MULTI
{

struct Evaluator
{

    PERMU_MULTI::params_multi *parameters;

    __net_eval_decl Evaluator(){};

    // fitness function in sequential order
    __net_eval_decl double FitnessFunction(NEAT::CpuNetwork *net, int n_evals, int initial_seed, int instance_index)
    {
        int seed_seq = initial_seed;
        params_multi parameters_tmp = params_multi(*parameters);
        parameters_tmp.INSTANCE_PATH = parameters->INSTANCE_PATHS[instance_index];
        parameters_tmp.MAX_TIME_PSO = parameters->MAX_TIME_PSO_FOR_EACH_INSTANCE[instance_index];
        double res = FitnessFunction_permu(net, n_evals, seed_seq, &parameters_tmp);
        seed_seq += n_evals;
        return res;
    }

    // parallelize over the same network
    __net_eval_decl void FitnessFunction_parallel(NEAT::CpuNetwork *net, int n_evals, double *res, int initial_seed, int instance_index)
    {
        int seed_parallel = initial_seed;
        params_multi parameters_tmp = params_multi(*parameters);
        parameters_tmp.INSTANCE_PATH = parameters->INSTANCE_PATHS[instance_index];
        parameters_tmp.MAX_TIME_PSO = parameters->MAX_TIME_PSO_FOR_EACH_INSTANCE[instance_index];
#pragma omp parallel for num_threads(N_OF_THREADS)
        for (int i = 0; i < n_evals; i++)
        {
            res[i] = FitnessFunction_permu(net, parameters->N_EVALS, seed_parallel + i, &parameters_tmp);
        }
        seed_parallel += n_evals;
    }

    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets)
    {
        NEAT::CpuNetwork **nets = (NEAT::CpuNetwork **)nets_;
        double progress_print_decider = 0.0;
        double **f_values = new double *[parameters->N_OF_INSTANCES]; //[instance_index][network_index]
        for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
        {
            f_values[i] = new double[nnets];
        }

        bool printed_bracket = false;
        RandomNumberGenerator rng;
        rng.seed();
        int initial_seed = rng.random_integer_fast(10050000, 20000000);
        cout << "Evaluating -> ";
        progress_bar bar = progress_bar(nnets);
        #pragma omp parallel for num_threads(N_OF_THREADS)
        for (size_t inet = 0; inet < nnets; inet++)
        {
            bar.step();
            NEAT::CpuNetwork *net = nets[inet];
            NEAT::OrganismEvaluation eval;
           
            for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
            {
                f_values[i][inet] = this->FitnessFunction(net, parameters->N_EVALS, initial_seed, i);
            }
            results[inet] = eval;
        }
        bar.end();

        // // reevaluate top n_of_threads_omp, with a minimum of 5 and a maximum of nnets.
        // double cut_value = obtain_kth_largest_value(f_values, min(max(n_of_threads_omp, 5), static_cast<int>(nnets)), nnets);

        // reevaluate top 5% at least N_REEVAL times
        int n_of_networks_to_reevaluate = nnets / 20 + 1;
        int n_of_reevals_top_5percent = parameters->N_EVALS * parameters->N_REEVALS_TOP_5_PERCENT;
        cout << "reevaluating top 5% (" << n_of_networks_to_reevaluate << " nets out of " << static_cast<int>(nnets) << ") each " << n_of_reevals_top_5percent << " times -> ";

        double *f_value_rankings = new double[nnets];

        for (int i = 0; i < (int)nnets; i++)
        {
            f_value_rankings[i] = 0.0;
        }

        for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
        {
            compute_order_from_double_to_double(
                f_values[i],
                nnets,
                f_values[i],
                false,
                true);
            sum_arrays(f_value_rankings, f_value_rankings, f_values[i], nnets);
        }

        double cut_value = obtain_kth_largest_value(f_value_rankings, n_of_networks_to_reevaluate+1, static_cast<int>(nnets));
        rng.seed();
        initial_seed = rng.random_integer_fast(20050000, 30000000);

        std::vector<int> reeval_indexes; 
        for (size_t inet = 0; inet < nnets; inet++)
        {   
            if (f_value_rankings[inet] <= cut_value)
            {
                for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
                {
                    f_values[i][inet] -= 1000000000.0;
                }
            }
            else
            {
                reeval_indexes.push_back(inet);
            }
        }
        
        bar.restart(reeval_indexes.size());
        #pragma omp parallel for num_threads(N_OF_THREADS)
        for (std::size_t i = 0; i < reeval_indexes.size(); ++i)
        {
            bar.step();
            f_value_rankings[reeval_indexes[i]] = 0.0;
            NEAT::CpuNetwork *net = nets[reeval_indexes[i]];
            for (int j = 0; j < parameters->N_OF_INSTANCES; j++)
            {
                f_values[j][i] = this->FitnessFunction(net, n_of_reevals_top_5percent, initial_seed, j);
            }
        }
        bar.end();

        for (int i = 0; i < (int)nnets; i++)
        {
            f_value_rankings[i] = 0.0;
        }
        for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
        {
            compute_order_from_double_to_double(
                f_values[i],
                nnets,
                f_values[i],
                false,
                true);
            sum_arrays(f_value_rankings, f_value_rankings, f_values[i], nnets);
        }

        cout << "Reevaluating best indiv of generation -> "<< std::flush;
        int index_most_fit = argmax(f_value_rankings, nnets);
        f_value_rankings[index_most_fit] += 1.0;
        NEAT::CpuNetwork *net = nets[index_most_fit];

        // apply a discount to all but the best individual
        for (int inet = 0; inet < (int)nnets; inet++)
        {
            if (inet != index_most_fit)
            {
                for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
                {
                    f_values[i][inet] -= 1000000000.0;
                }
            }
        }

        double **res = new double *[parameters->N_OF_INSTANCES];
        bar.restart(parameters->N_OF_INSTANCES);
        for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
        {
            bar.step();
            res[i] = new double[parameters->N_EVALS_TO_UPDATE_BK];
            this->FitnessFunction_parallel(net, parameters->N_EVALS_TO_UPDATE_BK, res[i], 30050000, i);
        }
        bar.end();

        // double median = res[arg_element_in_centile_specified_by_percentage(res, N_EVALS_TO_UPDATE_BK, 0.5)];

        bool is_it_better_in_all_cases = true;
        for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
        {
            is_it_better_in_all_cases = is_it_better_in_all_cases &&
                                        Average(res[i], parameters->N_EVALS_TO_UPDATE_BK) >= parameters->BEST_FITNESS_TRAIN_FOR_EACH_INSTANCE[i];
        }

        cout << "best this gen:";
        for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
        {
            cout << " " << Average(res[i], parameters->N_EVALS_TO_UPDATE_BK);
        }
        cout << endl;

        if (is_it_better_in_all_cases)
        {

            bool update_needed = false;

            for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
            {
                update_needed = update_needed || is_A_larger_than_B_Mann_Whitney(res[i], parameters->F_VALUES_OBTAINED_BY_BEST_INDIV_FOR_EACH_INSTANCE[i], parameters->N_EVALS_TO_UPDATE_BK);
            }

            if (update_needed)
            {
                N_TIMES_BEST_FITNESS_IMPROVED_TRAIN++;
                cout << "[BEST_FITNESS_IMPROVED] --> ";
                for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
                {
                    cout << " " << Average(res[i], parameters->N_EVALS_TO_UPDATE_BK);
                    parameters->BEST_FITNESS_TRAIN_FOR_EACH_INSTANCE[i] = Average(res[i], parameters->N_EVALS_TO_UPDATE_BK);
                    copy_vector(parameters->F_VALUES_OBTAINED_BY_BEST_INDIV_FOR_EACH_INSTANCE[i], res[i], parameters->N_EVALS_TO_UPDATE_BK);
                }
                cout << endl;
            }
        }

        for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
        {
            delete[] res[i];
        }
        delete[] res;


        double *tmp_order = new double[nnets];

        cout << "fitness_array: " << std::flush;
        //PrintArray(f_values, nnets);

        compute_order_from_double_to_double(f_value_rankings, nnets, f_value_rankings, false, true);

        multiply_array_with_value(f_value_rankings, 1.0 / (double)(nnets - 1), nnets);
        multiply_array_with_value(f_value_rankings, 1.0 + ((double)N_TIMES_BEST_FITNESS_IMPROVED_TRAIN / 1000.0), nnets);

        cout << "fitness_array: " << std::flush;
        //PrintArray(f_values, nnets);

        // save scaled fitness
        for (size_t inet = 0; inet < nnets; inet++)
        {
            results[inet].fitness = f_value_rankings[inet];
            results[inet].error = 2 - f_value_rankings[inet];
        }
        delete[] tmp_order;
        for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
        {
            delete[] f_values[i];
        }
        delete[] f_values;
        delete[] f_value_rankings;
    }
};

} //namespace PERMU_MULTI

namespace NEAT{

class Permu_multiEvaluator : public NetworkEvaluator
{
    NetworkExecutor<PERMU_MULTI::Evaluator> *executor;
    PERMU_MULTI::params_multi *parameters;

public:
    Permu_multiEvaluator()
    {
        executor = NEAT::NetworkExecutor<PERMU_MULTI::Evaluator>::create();
        parameters = new PERMU_MULTI::params_multi();
    }

    ~Permu_multiEvaluator()
    {
        delete executor;
    }

    virtual void execute(class NEAT::Network **nets_,
                         class NEAT::OrganismEvaluation *results,
                         size_t nnets)
    {
        NEAT::env->pop_size = POPSIZE_NEAT;
        PERMU_MULTI::Evaluator *ev = new PERMU_MULTI::Evaluator();
        ev->parameters = this->parameters;
        ev->execute(nets_, results, nnets);
        delete ev;
    }

    virtual void run_given_conf_file(std::string conf_file_path)
    {
        using namespace std;
        using namespace NEAT;
        using namespace PERMU_MULTI;

        INIReader reader(conf_file_path);


        if (reader.ParseError() != 0)
        {
            std::cout << "Can't load " << conf_file_path << "\n";
            exit(1);
        }

        string MODE = reader.Get("Global", "MODE", "UNKNOWN");
        string prob_name = reader.Get("Global", "PROBLEM_NAME", "UNKNOWN");


        if (MODE == "train")
        {

            parameters->N_EVALS = reader.GetInteger("NEAT", "N_EVALS", -1);
            parameters->N_REEVALS_TOP_5_PERCENT = reader.GetInteger("NEAT", "N_REEVALS_TOP_5_PERCENT", -1);
            parameters->N_EVALS_TO_UPDATE_BK = reader.GetInteger("NEAT", "N_EVALS_TO_UPDATE_BK", -1);
            string search_type = reader.Get("NEAT", "SEARCH_TYPE", "UNKOWN");
            parameters->PROBLEM_TYPE = reader.Get("Controller", "PROBLEM_TYPE", "UNKOWN");
            parameters->N_OF_INSTANCES = reader.GetInteger("Controller", "N_PROBLEMS", -1);
            parameters->INSTANCE_PATHS = new std::string[parameters->N_OF_INSTANCES];
            parameters->BEST_FITNESS_TRAIN_FOR_EACH_INSTANCE = new double[parameters->N_OF_INSTANCES];
            parameters->MAX_TIME_PSO_FOR_EACH_INSTANCE = new double[parameters->N_OF_INSTANCES];






            for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
            {
                parameters->INSTANCE_PATHS[i] = reader.Get("Controller", "PROBLEM_PATH_" + to_string(i), "UNKOWN");
                parameters->MAX_TIME_PSO_FOR_EACH_INSTANCE[i] = reader.GetReal("Controller", "MAX_TIME_PSO_FOR_EACH_INSTANCE_" + to_string(i), -1);
            }

            parameters->POPSIZE = reader.GetInteger("Controller", "POPSIZE", -1);
            parameters->TABU_LENGTH = reader.GetInteger("Controller", "TABU_LENGTH", -1);

            EXPERIMENT_FOLDER_NAME = "controllers_trained_with";
            cout << "Learning from instances: ";
            for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
            {
                EXPERIMENT_FOLDER_NAME += "_" + from_path_to_filename(parameters->INSTANCE_PATHS[i]);
                cout << from_path_to_filename(parameters->INSTANCE_PATHS[i]) << " ";
            }
            cout << endl;

            if (search_type == "phased")
            {
                env->search_type = GeneticSearchType::PHASED;
            }
            else if (search_type == "blended")
            {
                env->search_type = GeneticSearchType::BLENDED;
            }
            else if (search_type == "complexify")
            {
                env->search_type = GeneticSearchType::COMPLEXIFY;
            }
            else
            {
                cout << "Error, no search type specified." << endl;
            }

            if (N_OF_THREADS < 0)
            {
                cout << "please specify a valid number of threads on the conf. file" << endl;
                exit(1);
            }
            delete_prev_exp_folder();

            omp_set_num_threads(N_OF_THREADS);

            if (N_OF_THREADS < 7)
            {
                cout << "Warning: a minimum of 7 cores (specified by the THREADS parameter)"
                     << "is recommended for this implementation of NEAT to function correctly in a reasonable time." << endl;
            }

            if (env->pop_size < 500)
            {
                cout << "Warning: The population size of the controllers might be too low." << endl;
                cout << "The provided population size of the controllers is "
                     << env->pop_size << ", a value of at least 500 is recommended." << endl;
                cout << endl
                     << endl;
            }

            if (env->search_type == GeneticSearchType::BLENDED)
            {
                env->mutate_delete_node_prob *= 0.1;
                env->mutate_delete_link_prob *= 0.1;
            }

            parameters->F_VALUES_OBTAINED_BY_BEST_INDIV_FOR_EACH_INSTANCE = new double *[parameters->N_OF_INSTANCES];
            for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
            {
                parameters->F_VALUES_OBTAINED_BY_BEST_INDIV_FOR_EACH_INSTANCE[i] = new double[parameters->N_EVALS_TO_UPDATE_BK];
                for (int j = 0; j < parameters->N_EVALS_TO_UPDATE_BK; j++)
                {
                    parameters->F_VALUES_OBTAINED_BY_BEST_INDIV_FOR_EACH_INSTANCE[i][j] = -DBL_MAX;
                }
            }

            for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
            {
                parameters->BEST_FITNESS_TRAIN_FOR_EACH_INSTANCE[i] = -DBL_MAX;
            }

            
            Experiment *exp = Experiment::get(prob_name.c_str());
            int rng_seed = 2;
            rng_t rng{rng_seed};
            global_timer.tic();
            exp->run(rng);

            for (int i = 0; i < parameters->N_OF_INSTANCES; i++)
            {
                delete[] parameters->F_VALUES_OBTAINED_BY_BEST_INDIV_FOR_EACH_INSTANCE[i];
            }
            delete[] parameters->F_VALUES_OBTAINED_BY_BEST_INDIV_FOR_EACH_INSTANCE;
            delete[] parameters->INSTANCE_PATHS;
            delete[] parameters->BEST_FITNESS_TRAIN_FOR_EACH_INSTANCE;
            delete[] F_VALUES_OBTAINED_BY_BEST_INDIV;
            delete[] parameters->MAX_TIME_PSO_FOR_EACH_INSTANCE;

            return;
        }
        else
        {
            cout << "invalid mode provided. Please, use the configuration file to specify either test or train." << endl;
            exit(1);
        }
    }
};



class NetworkEvaluator *create_permu_multi_evaluator()
{
    return new Permu_multiEvaluator();
}

} // namespace NEAT
