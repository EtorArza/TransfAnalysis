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
#include <cfloat>
#include "INIReader.h"
#include "loadnetworkfromfile.h"
#include <omp.h>
#include "Population.h"
#include "PBP.h"
#include "QAP.h"
#include "LOP.h"
#include "PFSP.h"
#include "TSP.h"
#include "experiment.h"
#include "neat.h"
#include "rng.h"
#include "util.h"
#include <iomanip>
#include "Parameters.h"
#include "PERMU_params.h"
#include "constants.h"
#include <functional>
#include <vector>
#include <sstream>

using namespace std;

//#define PRINT
//#define RANDOM_SEARCH

namespace PERMU
{

void make_output_behaviour_mapping_more_injective(double *output)
{
    const int POS_FIRS_OPERATOR = 1;
    if (output[0] < -CUTOFF_0 || output[0] > CUTOFF_0)
    {
        output[0] = (double)move_to_0_minusone_or_one(output[0]);
    }
    else
    {
        for (int i = 0; i < PERMU::__output_N; i++)
        {
            output[i] = 0.0;
        }
        return;
    }

    if (sum_abs_val_slice_vec(output, 1, 1 + PERMU::N_OPERATORS) < SMALLEST_POSITIVE_DOUBLE)
    {
        for (int i = 0; i < PERMU::__output_N; i++)
        {
            output[i] = 0.0;
        }
        return;
    }

    if (output[PERMU::RANDOM_REINITIALIZE] > CUTOFF_0)
    {
        double R_or_clone_output = move_to_0_minusone_or_one(output[PERMU::REMOVE_OR_CLONE]);
        for (int i = 0; i < PERMU::__output_N; i++)
        {
            output[i] = 0.0;
        }
        output[PERMU::REMOVE_OR_CLONE] = R_or_clone_output;
        output[PERMU::RANDOM_REINITIALIZE] = 1.0;
        return;
    }
    else{
        output[PERMU::RANDOM_REINITIALIZE] = 0.0;
    }



    output[PERMU::TABU] = abs((double)move_to_0_minusone_or_one(output[PERMU::TABU]));
    output[PERMU::RELATIVE_TABU_SIZE] = 0.0;
    output[PERMU::REMOVE_OR_CLONE] = move_to_0_minusone_or_one(output[PERMU::REMOVE_OR_CLONE]);

    int nonzero_index = argmax(output + POS_FIRS_OPERATOR, PERMU::N_OPERATORS) + POS_FIRS_OPERATOR;
    for (int i = POS_FIRS_OPERATOR; i < POS_FIRS_OPERATOR + PERMU::N_OPERATORS; i++)
    {
        output[i] = 0.0;
    }
    output[nonzero_index + POS_FIRS_OPERATOR] = 1.0;

    if (output[0] < -CUTOFF_0) // local search
    {
        for (int i = 0; i < PERMU::N_PERMU_REFS; i++)
        {
            output[PERMU::__output_N - 1 - i] = 0.0;
        }
        output[accept_or_reject_worse] = 0.0;
    }
    else // movement
    {
        int pos_first_coef = PERMU::__output_N - PERMU::N_PERMU_REFS;
        double sum_of_coef = sum_abs_val_slice_vec(output, pos_first_coef, PERMU::__output_N);
        if (sum_of_coef > SMALLEST_POSITIVE_DOUBLE)
        {
            multiply_array_with_value(output + pos_first_coef, 1.0 / sum_of_coef, PERMU::N_PERMU_REFS);
        }
    }
}

struct Evaluator
{

    params *parameters;
    NEAT::CpuNetwork *best_network;
    int iteration_number = 0;

    __net_eval_decl Evaluator(){};


    // fitness function in sequential order
    __net_eval_decl double FitnessFunction(NEAT::CpuNetwork *net, int n_evals, int initial_seed)
    {
        int seed_seq = initial_seed;
        double res = FitnessFunction_permu(net, n_evals, seed_seq, parameters);
        seed_seq += n_evals;
        return res;
    }

    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets)
    {
        using namespace PERMU;
        NEAT::CpuNetwork **nets = (NEAT::CpuNetwork **)nets_;
        double **f_values = new double *[nnets];
        RandomNumberGenerator rng;

        for (size_t i = 0; i < nnets; i++)
        {
            f_values[i] = new double[MAX_EVALS_PER_CONTROLLER_LAST_IT + EVAL_MIN_STEP];
        }

        double *tmp_order = new double[nnets];
        // evaluate the individuals

        if (best_network == nullptr)
        {
            best_network = new NEAT::CpuNetwork(*nets[0]);
        }


        int current_n_of_evals = 0;
        int target_n_controllers_left;
        int max_evals_per_controller;
        if (parameters->neat_params->IS_LAST_ITERATION)
        {
            target_n_controllers_left = 1;
            max_evals_per_controller = MAX_EVALS_PER_CONTROLLER_LAST_IT;
        }
        else
        {
            target_n_controllers_left = (int)((double)nnets * NEAT::env->survival_thresh);
            max_evals_per_controller = MAX_EVALS_PER_CONTROLLER_REGULAR_IT;
        }
        

        vector<int> surviving_candidates;

        for (int inet = 0; inet < nnets; inet++)
        {
            surviving_candidates.push_back(inet);
        }

        while (surviving_candidates.size() > target_n_controllers_left && max_evals_per_controller > current_n_of_evals)
        {
            int initial_seed = rng.random_integer_uniform(INT_MAX);
            cout << "Evaluating -> " << std::flush;
            progress_bar bar(surviving_candidates.size());
            #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS)
            for (int i = 0; i < surviving_candidates.size() * EVAL_MIN_STEP; i++)
            {
                int inet = surviving_candidates[i / EVAL_MIN_STEP];
                NEAT::CpuNetwork *net = nets[inet];
                int seed = initial_seed + i % EVAL_MIN_STEP;
                f_values[inet][current_n_of_evals + i % EVAL_MIN_STEP] = this->FitnessFunction(net, 1, seed);
                cout << inet << "|" << current_n_of_evals + i % EVAL_MIN_STEP << "|" << seed << endl;
            }
            bar.end();
            cout << ", ";
            current_n_of_evals += EVAL_MIN_STEP;

            for (auto &&inet : surviving_candidates)
            {
                tmp_order[inet] = Average(f_values[inet], current_n_of_evals) - (double)surviving_candidates.size() * 10000000.0;
            }

            F_race_iteration(f_values, surviving_candidates, current_n_of_evals);


            cout << ", perc_discarded: " << (double)(nnets - surviving_candidates.size()) / (double)(nnets);
            cout << ", " << surviving_candidates.size() << " left.";
            cout << endl;
        }


        for (auto &&inet : surviving_candidates)
        {
            tmp_order[inet] = Average(f_values[inet], current_n_of_evals) - (double)surviving_candidates.size() * 10000000.0;
        }

        double avg_perf_best = 0;
        int initial_seed = rng.random_integer_uniform(INT_MAX);

        #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS)
        for (int i = 0; i < current_n_of_evals; i++)
        {
            avg_perf_best += this->FitnessFunction(best_network, 1, initial_seed + i) / (double) current_n_of_evals;
        }

        parameters->neat_params->BEST_FITNESS_TRAIN = (avg_perf_best + parameters->neat_params->BEST_FITNESS_TRAIN) / 2;
        double best_f_gen = Average(f_values[argmax(tmp_order, (int)nnets)], current_n_of_evals);
        cout << "(best this gen, best last gen) -> (" << best_f_gen << ", " << parameters->neat_params->BEST_FITNESS_TRAIN << ")";

        if (best_f_gen > parameters->neat_params->BEST_FITNESS_TRAIN || parameters->neat_params->IS_LAST_ITERATION)
        {
            parameters->neat_params->N_TIMES_BEST_FITNESS_IMPROVED_TRAIN++;
            cout << ", best replaced";
            parameters->neat_params->BEST_FITNESS_TRAIN = best_f_gen;
            delete best_network;
            best_network = new NEAT::CpuNetwork(*nets[argmax(tmp_order, (int)nnets)]);
            tmp_order[argmax(tmp_order, (int)nnets)] += 10000000.0;
        }

        cout << endl;



        compute_order_from_double_to_double(tmp_order, nnets, tmp_order, false, true);



        multiply_array_with_value(tmp_order, 1.0 / (double)(nnets - 1), (int)nnets);
        multiply_array_with_value(tmp_order, 1.0 + ((double)parameters->neat_params->N_TIMES_BEST_FITNESS_IMPROVED_TRAIN / 1000.0), (int)nnets);



        if (parameters->neat_params->IS_LAST_ITERATION)
        {

            cout << "fitness_matrix: " << std::flush;
            PrintMatrix(f_values, nnets, 32);


            cout << endl;
            cout << "tmp_order: " << std::flush;
            PrintArray(tmp_order, nnets);

            cout << endl;
            cout << "BEST_FITNESS_DEBUG_LAST_IT: " << this->FitnessFunction(best_network, 50, 2783492779) << endl;
            
            cout << endl;
        }
        // if (*is_last_gen)
        // {
        //     int argbest = argbest_net(nets_, nnets, 0.8);
        //     f_values[argbest] += 1.0;
        // }

        // save scaled fitness
        for (size_t inet = 0; inet < nnets; inet++)
        {
            NEAT::OrganismEvaluation eval;
            results[inet] = eval;
            results[inet].fitness = tmp_order[inet];
            results[inet].error = 2 - tmp_order[inet];
        }

        for (size_t i = 0; i < nnets; i++)
        {
            delete[] f_values[i];
        }
        delete[] f_values;

        delete[] tmp_order;
    }
};

} // namespace PERMU

namespace NEAT
{

PermuEvaluator::PermuEvaluator()
{
    executor = NEAT::NetworkExecutor<PERMU::Evaluator>::create();
    parameters = new PERMU::params();
    best_network = nullptr;
    iteration_number = 0;
}

PermuEvaluator::~PermuEvaluator()
{
    delete executor;
}

void PermuEvaluator::read_conf_file(std::string conf_file_path)
{
    using namespace std;
    using namespace NEAT;

    INIReader reader(conf_file_path);

    if (reader.ParseError() != 0)
    {
        std::cout << "Can't load " << conf_file_path << "\n";
        exit(1);
    }

    parameters->MODE = reader.Get("Global", "MODE", "UNKNOWN");
    parameters->prob_name = reader.Get("Global", "PROBLEM_NAME", "UNKNOWN");

    if (parameters->MODE == "train")
    {

        parameters->SEED = reader.GetInteger("Global", "SEED", -1);
        parameters->N_EVALS = reader.GetInteger("Global", "N_EVALS", -1);
        string search_type = reader.Get("Global", "SEARCH_TYPE", "UNKOWN");
        parameters->PROBLEM_TYPE = reader.Get("Global", "PROBLEM_TYPE", "UNKOWN");
        parameters->INSTANCE_PATH = reader.Get("Global", "PROBLEM_PATH", "UNKOWN");
        parameters->MAX_SOLVER_TIME = reader.GetReal("Global", "MAX_SOLVER_TIME", -1.0);

        if (parameters->SEED == -1)
        {
            RandomNumberGenerator tmp_rng;
            parameters->SEED = tmp_rng.random_integer_uniform(INT_MAX);
        }
        

        if (neat_params->EXPERIMENT_FOLDER_NAME == "UNKNOWN")
        {
            neat_params->EXPERIMENT_FOLDER_NAME = "controllers_trained_with_" + from_path_to_filename(parameters->INSTANCE_PATH);
        }
        
        cout << "Learning from instance: " << from_path_to_filename(parameters->INSTANCE_PATH) << endl;

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

        if (neat_params->N_OF_THREADS < 0)
        {
            cout << "please specify a valid number of threads on the conf. file" << endl;
            exit(1);
        }

        neat_params->delete_prev_exp_folder();

        omp_set_num_threads(neat_params->N_OF_THREADS);

        if (neat_params->N_OF_THREADS < 7)
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

        return;
    }
    else if (parameters->MODE == "test")
    {

        //const char * prob_name = "permu";
        //Experiment *exp = Experiment::get(prob_name);

        parameters->PROBLEM_TYPE = reader.Get("Global", "PROBLEM_TYPE", "UNKOWN");
        parameters->INSTANCE_PATH = reader.Get("Global", "PROBLEM_PATH", "UNKOWN");
        parameters->MAX_SOLVER_TIME = reader.GetReal("Global", "MAX_SOLVER_TIME", -1.0);
        parameters->CONTROLLER_PATH = reader.Get("Global", "CONTROLLER_PATH", "UNKNOWN");
        parameters->N_REPS = reader.GetInteger("Global", "N_REPS", -1);
        parameters->N_EVALS = reader.GetInteger("Global", "N_EVALS", -1);
        parameters->COMPUTE_RESPONSE = reader.GetBoolean("Global", "COMPUTE_RESPONSE", false);
        neat_params->N_OF_THREADS = std::min(neat_params->N_OF_THREADS, parameters->N_EVALS);

        if (parameters->CONTROLLER_PATH == "UNKNOWN")
        {
            cout << "error, controller path not specified in test." << endl;
        }

        if (parameters->N_REPS < 0)
        {
            cout << "error, N_REPS not provided in test mode." << endl;
        }

        return;
    }

    else
    {
        cout << "invalid mode provided. Please, use the configuration file to specify either test or train." << endl;
        exit(1);
    }
}

void PermuEvaluator::execute(class NEAT::Network **nets_, class NEAT::OrganismEvaluation *results, size_t nnets)
{
    using namespace NEAT;
    env->pop_size = this->neat_params->POPSIZE_NEAT;
    PERMU::Evaluator *ev = new PERMU::Evaluator();
    this->parameters->neat_params = this->neat_params;
    ev->parameters = this->parameters;
    ev->iteration_number = this->iteration_number;
    ev->best_network = this->best_network;
    ev->execute(nets_, results, nnets);

    this->best_network = ev->best_network;
    this->iteration_number++;
    delete ev;
}

void PermuEvaluator::run_given_conf_file(std::string conf_file_path)
{
    using namespace std;
    using namespace NEAT;

    read_conf_file(conf_file_path);
    parameters->neat_params = this->neat_params;

    if (parameters->MODE == "train")
    {

        Experiment *exp = Experiment::get(parameters->prob_name.c_str());
        rng_t rng{parameters->SEED};
        exp->neat_params->global_timer.tic();
        exp->run(rng);
        return;
    }
    else if (parameters->MODE == "test")
    {

        CpuNetwork net = load_network(parameters->CONTROLLER_PATH);

        if (parameters->COMPUTE_RESPONSE)
        {
            net.start_recording_response(PERMU::make_output_behaviour_mapping_more_injective);
        }

        double *v_of_f_values = new double[parameters->N_EVALS];

        cout << std::setprecision(15);
        RandomNumberGenerator *rng;
        rng = new RandomNumberGenerator();
        rng->seed();
        int initial_seed = rng->random_integer_uniform(40000000, 50000000);
        delete rng;
        ostringstream result_string_stream;

        result_string_stream << "[[";
        for (int j = 0; j < parameters->N_REPS; j++)
        {
#pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS)
            for (int i = 0; i < parameters->N_EVALS; i++)
            {
                v_of_f_values[i] = FitnessFunction_permu(&net, 1, initial_seed + i, parameters);
            }
            initial_seed += parameters->N_EVALS;
            double res = Average(v_of_f_values, parameters->N_EVALS);
            result_string_stream << res;
            if (j < parameters->N_REPS - 1)
            {
                result_string_stream << ",";
            }
        }
        result_string_stream << "]," << std::flush;
        delete[] v_of_f_values;

        result_string_stream << std::setprecision(15);
        result_string_stream << std::flush;
        result_string_stream << "\"" << parameters->INSTANCE_PATH << "\",\""
                             << parameters->CONTROLLER_PATH << "\",\""
                             << parameters->PROBLEM_TYPE << "\","
                             << parameters->N_EVALS
                             << "]"
                             << endl;

        if (parameters->COMPUTE_RESPONSE)
        {
            double *res = new double[PERMU::__output_N];
            net.return_average_response_and_stop_recording(res);
            append_line_to_file(
                "responses.txt",
                "['" + parameters->CONTROLLER_PATH + "', '" +
                    parameters->INSTANCE_PATH + "', " +
                    array_to_python_list_string(res, PERMU::__output_N) + "]\n");
            delete[] res;
        }

        // cout << res << std::endl;;
        result_string_stream << std::flush;

        string result_string = result_string_stream.str();

        append_line_to_file("result.txt", result_string);

        return;
    }
    else
    {
        cout << "invalid mode provided. Please, use the configuration file to specify either test or train." << endl;
        exit(1);
    }
}

class NetworkEvaluator *create_permu_evaluator()
{
    return new PermuEvaluator();
}

} // namespace NEAT
