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
#include <functional>
#include <vector>
#include <sstream>


using namespace std;

//#define PRINT
//#define RANDOM_SEARCH



namespace PERMU{
void make_output_behaviour_mapping_more_injective(double *output)
{
#define POS_FIRS_OPERATOR 1
    if (output[0] < -CUTOFF_0 || output[0] > CUTOFF_0)
    {
        output[0] = (double) move_to_0_minusone_or_one(output[0]);
    }
    else
    {
        for (int i = 0; i < PERMU::__output_N; i++)
        {
            output[i] = 0.0;
        }
        return;
    }

    if (sum_abs_val_slice_vec(output, 1, 1 + PERMU::N_OPERATORS) == 0)
    {
        for (int i = 0; i < PERMU::__output_N; i++)
        {
            output[i] = 0.0;
        }
        return;
    }

    output[PERMU::TABU] = (double) move_to_0_minusone_or_one(output[PERMU::TABU]);


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

    params* parameters;


    __net_eval_decl Evaluator(){};


    // fitness function in sequential order
    __net_eval_decl double FitnessFunction(NEAT::CpuNetwork *net, int n_evals, int initial_seed)
    {
        int seed_seq = initial_seed;
        double res = FitnessFunction_permu(net, n_evals, seed_seq, parameters);
        seed_seq += n_evals;
        return res;
    }

    // parallelize over the same network
    __net_eval_decl void FitnessFunction_parallel(NEAT::CpuNetwork *net, int n_evals, double *res, int initial_seed)
    {
        using namespace PERMU;
        int seed_parallel = initial_seed;

        #pragma omp parallel for num_threads(N_OF_THREADS)
        for (int i = 0; i < n_evals; i++)
        {
            res[i] = FitnessFunction_permu(net, parameters->N_EVALS, seed_parallel + i, parameters);
        }
        seed_parallel += n_evals;
    }

    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets){
        using namespace PERMU;
        NEAT::CpuNetwork **nets = (NEAT::CpuNetwork **)nets_;
        double *f_values = new double[nnets];
        RandomNumberGenerator rng;
        rng.seed();
        int initial_seed = rng.random_integer_fast(10050000,20000000);
        // evaluate the individuals


        cout << " Evaluating -> ";


        progress_bar bar(nnets);
        #pragma omp parallel for num_threads(N_OF_THREADS)
        for (int inet = 0; inet < nnets; inet++)
        {
            NEAT::CpuNetwork *net = nets[inet];
            NEAT::OrganismEvaluation eval;
            int seed = initial_seed;
            results[inet] = eval;
            // if (classes_array[inet] == inet) // copy fitness
            // { // copy fitness
                f_values[inet] = this->FitnessFunction(net, parameters->N_EVALS, seed);
            // } copy  fitness
            bar.step();
        }
        bar.end();

        // // Copy fitness value from repeated networks.
        // #pragma omp parallel for num_threads(N_OF_THREADS)
        // for (int inet = 0; inet < nnets; inet++)
        // {
        //     NEAT::CpuNetwork *net = nets[inet];
        //     NEAT::OrganismEvaluation eval;
        //     int seed = initial_seed;
        //     if (classes_array[inet] != inet)
        //     {
        //         f_values[inet] = f_values[classes_array[inet]];
        //     }
        // }



        // reevaluate top 5% at least N_REEVAL times
        int n_of_networks_to_reevaluate = nnets / 20 + 1;
        int n_of_reevals_top_5percent = parameters->N_EVALS * parameters->N_REEVALS_TOP_5_PERCENT;
        cout << "reevaluating top 5% (" << n_of_networks_to_reevaluate << " nets out of " << static_cast<int>(nnets) << ") each " << n_of_reevals_top_5percent << " times -> ";

        double cut_value = obtain_kth_largest_value(f_values, n_of_networks_to_reevaluate+1, static_cast<int>(nnets));

        rng.seed();
        initial_seed = rng.random_integer_fast(20050000,30000000);

        std::vector<int> reeval_indexes; 
        for (size_t inet = 0; inet < nnets; inet++)
        {
            if (f_values[inet] <= cut_value)
            {
                f_values[inet] -= 1000000000.0; // apply a discount to the individuals that are not reevaluated
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
            NEAT::CpuNetwork *net = nets[reeval_indexes[i]];
            f_values[reeval_indexes[i]] = this->FitnessFunction(net, n_of_reevals_top_5percent, initial_seed);
        }

        bar.end();
        cout << "Reevaluating best of gen: "<< std::flush;
        int index_most_fit = argmax(f_values, nnets);
        NEAT::CpuNetwork *net = nets[index_most_fit];

        // apply a discount to all but the best individual
        for (int i = 0; i < (int) nnets; i++)
        {
            if (i != index_most_fit)
            {
                f_values[i] -= 1000000000.0; 
            }
        }

        double *res = new double[parameters->N_EVALS_TO_UPDATE_BK];
        bar.restart(1);
        this->FitnessFunction_parallel(net, parameters->N_EVALS_TO_UPDATE_BK, res, 40500000);
        bar.end();

        // double median = res[arg_element_in_centile_specified_by_percentage(res, N_EVALS_TO_UPDATE_BK, 0.5)];
        double average = Average(res,parameters->N_EVALS_TO_UPDATE_BK);


        cout << "best this gen: " << average << endl;

        if (average > BEST_FITNESS_TRAIN)        {


            bool update_needed = is_A_larger_than_B_Mann_Whitney(res, F_VALUES_OBTAINED_BY_BEST_INDIV, parameters->N_EVALS_TO_UPDATE_BK);

            if (update_needed)
            {
                N_TIMES_BEST_FITNESS_IMPROVED_TRAIN++;
                cout << "[BEST_FITNESS_IMPROVED] --> " << average << endl;
                BEST_FITNESS_TRAIN = average;
                copy_array(F_VALUES_OBTAINED_BY_BEST_INDIV, res, parameters->N_EVALS_TO_UPDATE_BK);
            }
        }

        delete[] res;

        double* tmp_order = new double[nnets];

        //cout << "fitness_array: " << std::flush;
        //PrintArray(f_values, nnets);

        compute_order_from_double_to_double(f_values, nnets, tmp_order, false, true);

        std::swap(f_values, tmp_order);



        multiply_array_with_value(f_values, 1.0 / (double) (nnets-1), nnets);
        multiply_array_with_value(f_values, 1.0 + ((double)N_TIMES_BEST_FITNESS_IMPROVED_TRAIN / 1000.0), nnets);

        //cout << "fitness_array: " << std::flush;
        //PrintArray(f_values, nnets);

        // save scaled fitness
        for (size_t inet = 0; inet < nnets; inet++)
        {
            results[inet].fitness = f_values[inet];
            results[inet].error = 2 - f_values[inet];
        }
        delete[] tmp_order;
        delete[] f_values;
        delete[] classes_array;
    
    }
};

} // namespace PERMU


namespace NEAT
{


class PermuEvaluator : public NetworkEvaluator
{   
    NetworkExecutor<PERMU::Evaluator> *executor;
    PERMU::params* parameters;

public:
    PermuEvaluator()
    {   
        executor = NEAT::NetworkExecutor<PERMU::Evaluator>::create();
        parameters = new PERMU::params();
    }

    ~PermuEvaluator()
    {
        delete executor;
    }

    virtual void execute(class NEAT::Network **nets_,
                         class NEAT::OrganismEvaluation *results,
                         size_t nnets)
    {
        using namespace NEAT;
        env->pop_size = POPSIZE_NEAT;
        PERMU::Evaluator *ev = new PERMU::Evaluator();
        ev->parameters = this->parameters;
        ev->execute(nets_, results, nnets);
        delete ev;
    }

    virtual void run_given_conf_file(std::string conf_file_path){
    using namespace std;
    using namespace NEAT;


    INIReader reader(conf_file_path);

    if (reader.ParseError() != 0) {
        std::cout << "Can't load " << conf_file_path << "\n";
        exit(1);
    }

    string MODE = reader.Get("Global", "MODE", "UNKNOWN");
    std::string prob_name = reader.Get("Global", "PROBLEM_NAME", "UNKNOWN");


    if (MODE == "train")
    {

        int rng_seed = reader.GetInteger("NEAT","SEED", -1);
        parameters->N_EVALS = reader.GetInteger("NEAT", "N_EVALS", -1);
        parameters->N_REEVALS_TOP_5_PERCENT = reader.GetInteger("NEAT","N_REEVALS_TOP_5_PERCENT", -1);
        parameters->N_EVALS_TO_UPDATE_BK = reader.GetInteger("NEAT","N_EVALS_TO_UPDATE_BK", -1);
        string search_type = reader.Get("NEAT", "SEARCH_TYPE", "UNKOWN");
        parameters->PROBLEM_TYPE = reader.Get("Controller", "PROBLEM_TYPE", "UNKOWN");
        parameters->INSTANCE_PATH = reader.Get("Controller", "PROBLEM_PATH", "UNKOWN");
        parameters->MAX_TIME_PSO = reader.GetReal("Controller", "MAX_TIME_PSO", -1.0);
        parameters->POPSIZE = reader.GetInteger("Controller", "POPSIZE", -1);
        parameters->TABU_LENGTH = reader.GetInteger("Controller", "TABU_LENGTH", -1);

        EXPERIMENT_FOLDER_NAME = "controllers_trained_with_" + from_path_to_filename(parameters->INSTANCE_PATH);

        cout << "Learning from instance: " << from_path_to_filename(parameters->INSTANCE_PATH) << endl;


    F_VALUES_OBTAINED_BY_BEST_INDIV = new double[parameters->N_EVALS_TO_UPDATE_BK];
    for (int i = 0; i < parameters->N_EVALS_TO_UPDATE_BK; i++)
    {
        F_VALUES_OBTAINED_BY_BEST_INDIV[i] = -DBL_MAX;
    }



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

        if(N_OF_THREADS < 0){
            cout << "please specify a valid number of threads on the conf. file" <<endl;
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
            cout << endl << endl;
        }




        if (env->search_type == GeneticSearchType::BLENDED)
        {
            env->mutate_delete_node_prob *= 0.1;
            env->mutate_delete_link_prob *= 0.1;
        }




        Experiment *exp = Experiment::get(prob_name.c_str());
        rng_t rng{rng_seed};
        global_timer.tic();
        exp->run(rng);


        delete[] F_VALUES_OBTAINED_BY_BEST_INDIV;

        return;
    }
    else if (MODE ==  "test")
    {

        //const char * prob_name = "permu";
        //Experiment *exp = Experiment::get(prob_name);

        parameters->PROBLEM_TYPE = reader.Get("Controller", "PROBLEM_TYPE", "UNKOWN");
        parameters->INSTANCE_PATH = reader.Get("Controller", "PROBLEM_PATH", "UNKOWN");
        parameters->MAX_TIME_PSO = reader.GetReal("Controller", "MAX_TIME_PSO", -1.0);
        parameters->POPSIZE = reader.GetInteger("Controller", "POPSIZE", -1);
        parameters->TABU_LENGTH = reader.GetInteger("Controller", "TABU_LENGTH", -1);
        parameters->CONTROLLER_PATH = reader.Get("TestSettings", "CONTROLLER_PATH", "UNKNOWN");
        parameters->N_REPS = reader.GetInteger("TestSettings", "N_REPS", -1);
        parameters->N_EVALS = reader.GetInteger("TestSettings", "N_EVALS", -1);
        parameters->COMPUTE_RESPONSE = reader.GetBoolean("TestSettings", "COMPUTE_RESPONSE", false);
        N_OF_THREADS = MIN(N_OF_THREADS, parameters->N_EVALS);


        if (parameters->CONTROLLER_PATH == "UNKNOWN")
        {
            cout << "error, controller path not specified in test." << endl;
        }

        if (parameters->N_REPS < 0)
        {
             cout << "error, N_REPS not provided in test mode." << endl;
        }
        
        
        CpuNetwork net = load_network(parameters->CONTROLLER_PATH);

        if (parameters->COMPUTE_RESPONSE)
        {
            net.start_recording_response(PERMU::make_output_behaviour_mapping_more_injective);
        }

        double *v_of_f_values = new double[parameters->N_EVALS];

        cout << std::setprecision(15);
        RandomNumberGenerator* rng;
        rng = new RandomNumberGenerator();
        rng->seed();
        int initial_seed = rng->random_integer_uniform(40000000, 50000000);
        delete rng;
        ostringstream result_string_stream;

        result_string_stream << "[[";
        for (int j = 0; j < parameters->N_REPS; j++)
        {
            #pragma omp parallel for num_threads(N_OF_THREADS)
            for (int i = 0; i < parameters->N_EVALS; i++)
            {
                v_of_f_values[i] = FitnessFunction_permu(&net, 1, initial_seed + i, parameters);
            }
            initial_seed += parameters->N_EVALS;
            double res = Average(v_of_f_values, parameters->N_EVALS);
            result_string_stream << res;
            if (j < parameters->N_REPS-1)
            {
                result_string_stream << ",";
            }
            
        }
        result_string_stream << "]," << std::flush;
        delete[] v_of_f_values;


        result_string_stream << std::setprecision(15);
        result_string_stream << std::flush;
        result_string_stream << parameters->INSTANCE_PATH   << ","
             << parameters->CONTROLLER_PATH << ","
             << parameters->PROBLEM_TYPE    << ","
             << parameters->N_EVALS
             << "]"
             << endl;

        if (parameters->COMPUTE_RESPONSE)
        {
            double* res = new double[PERMU::__output_N];
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

};

class NetworkEvaluator *create_permu_evaluator()
{
    return new PermuEvaluator();
}

} // namespace NEAT
