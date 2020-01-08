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

using namespace std;
using namespace PERMU;

//#define COUNTER
//#define PRINT
//#define RANDOM_SEARCH



namespace NEAT
{

struct Evaluator
{

    params* parameters;


    __net_eval_decl Evaluator(){};


    // fitness function in sequential order
    __net_eval_decl double FitnessFunction(CpuNetwork *net, int n_evals, int initial_seed)
    {
        int seed_seq = initial_seed;
        double res = FitnessFunction_permu(net, n_evals, seed_seq, parameters);
        seed_seq += n_evals;
        return res;
    }

    // parallelize over the same network
    __net_eval_decl void FitnessFunction_parallel(CpuNetwork *net, int n_evals, double *res, int initial_seed)
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
    __net_eval_decl void execute(class Network **nets_, OrganismEvaluation *results, size_t nnets){
        using namespace PERMU;
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
            OrganismEvaluation eval;
            int seed = initial_seed;
            f_values[inet] = this->FitnessFunction(net, parameters->N_EVALS, seed);
            results[inet] = eval;

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
        int actual_n_reevals = (((parameters->N_REEVALS_TOP_5_PERCENT - 1) / N_OF_THREADS) + 1) * N_OF_THREADS;
        int n_of_networks_to_reevaluate = MAX(1, static_cast<int>(nnets) * 5 / 100);
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
                double *res = new double[actual_n_reevals];
                int seed = initial_seed;
                this->FitnessFunction_parallel(net, actual_n_reevals, res, seed);
                int index_value = arg_element_in_centile_specified_by_percentage(res, actual_n_reevals, 0.50);
                f_values[inet] = res[index_value];
                delete[] res;
            }
        }

        cout << "Reevaluating best indiv of generation: ";
        int index_most_fit = argmax(f_values, nnets);
        CpuNetwork *net = nets[index_most_fit];

        // apply a discount to all but the best individual
        for (int i = 0; i < (int) nnets; i++)
        {
            if (i != index_most_fit)
            {
                f_values[i] -= 1000000000.0; 
            }
        }

        double *res = new double[parameters->N_EVALS_TO_UPDATE_BK];
        this->FitnessFunction_parallel(net, parameters->N_EVALS_TO_UPDATE_BK, res, 30050000);


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
                copy_vector(F_VALUES_OBTAINED_BY_BEST_INDIV, res, parameters->N_EVALS_TO_UPDATE_BK);
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
    
    }
};






class PermuEvaluator : public NetworkEvaluator
{   
    NetworkExecutor<Evaluator> *executor;
    PERMU::params* parameters;

public:
    PermuEvaluator()
    {   
        executor = NetworkExecutor<Evaluator>::create();
        parameters = new PERMU::params();
    }

    ~PermuEvaluator()
    {
        delete executor;
    }

    virtual void execute(class Network **nets_,
                         class OrganismEvaluation *results,
                         size_t nnets)
    {
        env->pop_size = POPSIZE_NEAT;
        Evaluator *ev = new Evaluator();
        ev->parameters = this->parameters;
        ev->execute(nets_, results, nnets);
    }

    virtual void run_given_conf_file(std::string conf_file_path){
    using namespace std;
    using namespace NEAT;
    using namespace PERMU;


    INIReader reader(conf_file_path);

    if (reader.ParseError() != 0) {
        std::cout << "Can't load " << conf_file_path << "\n";
        exit(1);
    }

    string MODE = reader.Get("Global", "MODE", "UNKNOWN");


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


        else if (exists(EXPERIMENT_FOLDER_NAME))
        {
            error("Already exists: " + EXPERIMENT_FOLDER_NAME + ".\nMove your experiment directories or use -f to delete them automatically. If -f is used, all previous experiments will be deleted.")
        }

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




        const char * prob_name = "permu";
        Experiment *exp = Experiment::get(prob_name);
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

        double *v_of_f_values = new double[parameters->N_EVALS];

        cout << std::setprecision(15);
        RandomNumberGenerator* rng;
        rng = new RandomNumberGenerator();
        rng->seed();
        int initial_seed = rng->random_integer_uniform(40000000, 50000000);
        delete rng;
        cout << "[[";
        for (int j = 0; j < parameters->N_REPS; j++)
        {
            #pragma omp parallel for num_threads(N_OF_THREADS)
            for (int i = 0; i < parameters->N_EVALS; i++)
            {
                v_of_f_values[i] = FitnessFunction_permu(&net, 1, initial_seed + i, parameters);
            }
            initial_seed += parameters->N_EVALS;
            double res = Average(v_of_f_values, parameters->N_EVALS);
            cout << res;
            if (j < parameters->N_REPS-1)
            {
                cout << ",";
            }
            
        }
        cout << "]," << std::flush;
;
        delete[] v_of_f_values;


        

        cout << std::setprecision(15);
        cout << std::flush;
        cout << parameters->INSTANCE_PATH   << ","
             << parameters->CONTROLLER_PATH << ","
             << parameters->PROBLEM_TYPE    << ","
             << parameters->N_EVALS
             << "]"
             << endl;


        // cout << res << std::endl;;
        cout << std::flush;

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
