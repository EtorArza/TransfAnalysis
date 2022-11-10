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



void operator++( PERMU::operator_t &c, int )
{
    int index = (int) c;
    index++;
    index %= PERMU::N_OPERATORS;
    PERMU::operator_t res;
    c = (PERMU::operator_t) index;
}

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
        for (int i = 0; i < PERMU::__output_N; i++)
        {
            output[i] = 0.0;
        }
        output[PERMU::RANDOM_REINITIALIZE] = 1.0;
        return;
    }
    else{
        output[PERMU::RANDOM_REINITIALIZE] = 0.0;
    }




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
    __net_eval_decl double FitnessFunction(NEAT::CpuNetwork *net, int seed)
    {
        double res = FitnessFunction_permu(net, seed, parameters);
        return res;
    }

    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets)
    {
        cout << "Need to call execute_multi() in experiment.cpp. See execute() in real_func_evaluator.cxx";
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
    parameters->SOLVER_POPSIZE = reader.GetInteger("Global", "SOLVER_POPSIZE", 8);

    if (parameters->MODE == "train")
    {

        string search_type = reader.Get("Global", "SEARCH_TYPE", "UNKOWN");
        parameters->PROBLEM_TYPE = reader.Get("Global", "PROBLEM_TYPE", "UNKOWN");
        parameters->INSTANCE_PATH = reader.Get("Global", "PROBLEM_PATH", "UNKOWN");
        parameters->MAX_SOLVER_TIME = reader.GetReal("Global", "MAX_SOLVER_TIME", 43200000.0);
        parameters->MAX_SOLVER_FE = reader.GetInteger("Global", "MAX_SOLVER_FE", LONG_MAX);

        if (parameters->SEED == -1)
        {
            RandomNumberGenerator tmp_rng;
            parameters->SEED = tmp_rng.random_integer();
        }

        if(parameters->MAX_SOLVER_TIME == 43200000.0 && parameters->MAX_SOLVER_FE == LONG_MAX)
        {
            cout << "ERROR: No stoping criterion for permu solver. Either MAX_SOLVER_TIME or MAX_SOLVER_FE is required." << endl;
            exit(1);
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
        parameters->MAX_SOLVER_TIME = reader.GetReal("Global", "MAX_SOLVER_TIME", 43200000.0);
        parameters->MAX_SOLVER_FE = reader.GetInteger("Global", "MAX_SOLVER_FE", INT_MAX);
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
        uint32_t initial_seed = neat_params->SEED;
        ostringstream result_string_stream;
        result_string_stream << std::setprecision(15);
        result_string_stream << std::flush;

        result_string_stream << "[[";
        for (int j = 0; j < parameters->N_REPS; j++)
        {
#pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS)
            for (int i = 0; i < parameters->N_EVALS; i++)
            {
                v_of_f_values[i] = FitnessFunction_permu(&net, initial_seed + i, parameters);
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

        append_line_to_file("score.txt", result_string);

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
