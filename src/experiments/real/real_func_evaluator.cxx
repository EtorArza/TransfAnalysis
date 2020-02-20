#include "std.h" // Must be included first. Precompiled header with standard library includes.
#include "std.hxx"
#include "Parameters.h"
#include "REAL_FUNC_params.h"
#include "real_func_evaluator.h"
#include "map.h"
#include "network.h"
#include "networkexecutor.h"
#include "resource.h"
#include <assert.h>
#include "MultidimBenchmarkFF.h"
#include "real_func_src/Population.h"
#include "Tools.h"
#include <iomanip>
#include "INIReader.h"
#include "neat.h"
#include <cfloat>
#include "util.h"
#include <omp.h>
#include "experiment.h"
#include "rng.h"
#include "loadnetworkfromfile.h"

using namespace std;
// #define COUNTER

MultidimBenchmarkFF *load_problem(int problem_index, int dim)
{
    MultidimBenchmarkFF *problem;
    switch (problem_index)
    {
    case 1:
        problem = new F1(dim);
        break;
    case 2:
        problem = new F2(dim);
        break;
    case 3:
        problem = new F3(dim);
        break;
    case 4:
        problem = new F4(dim);
        break;
    case 5:
        problem = new F5(dim);
        break;
    case 6:
        problem = new F6(dim);
        break;
    case 7:
        problem = new F7(dim);
        break;
    case 8:
        problem = new F8(dim);
        break;
    default:
        cout << "Incorrect problem index, only integers between 1 and 8 allowed. problem_index = " << problem_index << "  was provided." << endl;
        std::exit(1);
        break;
    }
    return problem;
}

double FitnessFunction_real_func(class NEAT::CpuNetwork *net_original, int problem_index, int dim, int n_evals, int seed, REAL_FUNC::params *parameters)
{



    double *v_of_fitness;
    MultidimBenchmarkFF *problem = load_problem(problem_index, dim);

    CPopulation *pop;

    NEAT::CpuNetwork tmp_net = *net_original;
    NEAT::CpuNetwork *net = &tmp_net;

    double best_first_it;

    pop = new CPopulation(problem, parameters);
    problem->load_rng(pop->rng);
    pop->rng->seed(seed);

    v_of_fitness = new double[n_evals];

    for (int i = 0; i < parameters->POPSIZE; i++)
    {
        pop->m_individuals[i]->activation = std::vector<double>(net->activations);
    }

#ifdef COUNTER
    int counter = 0;
#endif

    for (int n_of_repetitions_completed = 0; n_of_repetitions_completed < n_evals; n_of_repetitions_completed++)
    {

#ifdef COUNTER
        counter = 0;
#endif
        pop->rng->seed(seed + n_of_repetitions_completed);
        pop->Reset();
        best_first_it = pop->f_best;
        //std::cout << "|" << n_of_repetitions_completed << "|" << std::endl;
        for (int i = 0; i < pop->popsize; i++)
        {
            std::swap(net->activations, pop->m_individuals[i]->activation);
            net->clear_noninput();
            std::swap(net->activations, pop->m_individuals[i]->activation);
        }
        while (!pop->terminated)
        {
#ifdef COUNTER
            counter++;
// if (counter < 3 || counter == 50)
// {
//     std::cout << "iteration number: " << counter << std::endl;
// }
#endif

            if (parameters->PRINT_POSITIONS)
            {
                pop->print_positions("positions.txt");
            }

            for (int i = 0; i < pop->popsize; i++)
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
            //pop->Print();
        }

        if (problem->Fitness_Func_0_1(pop->genome_best) > 0.000001)
        {
            cout << "Error, positive fitness detected.";
            std::exit(1);
        }

        v_of_fitness[n_of_repetitions_completed] = problem->Fitness_Func_0_1(pop->genome_best); // - total_discount_clipping;
        net->clear_noninput();
#ifdef COUNTER
        cout << counter << endl;
#endif
    }

    double res = Average(v_of_fitness, n_evals);

    delete[] v_of_fitness;
    delete pop;
    delete problem;
    pop = NULL;
    v_of_fitness = NULL;
    problem = NULL;
    net = NULL;
    return res;
}

double FitnessFunction_real_func(NEAT::CpuNetwork *net_original, int seed, int n_evals, REAL_FUNC::params *parameters)
{
    return FitnessFunction_real_func(net_original, parameters->PROBLEM_INDEX, parameters->PROBLEM_DIMENSIONS, n_evals, seed, parameters);
}

namespace REAL_FUNC
{

    struct Evaluator
    {
        REAL_FUNC::params *parameters;

        __net_eval_decl Evaluator(){};

        // fitness function in sequential order
        __net_eval_decl double FitnessFunction(NEAT::CpuNetwork *net, int n_evals, int initial_seed)
        {
            int seed_seq = initial_seed;
            double res = FitnessFunction_real_func(net, seed_seq, n_evals, parameters);
            seed_seq += n_evals;
            return res;
        }

        // parallelize over the same network
        __net_eval_decl void FitnessFunction_parallel(NEAT::CpuNetwork *net, int n_evals, double *res, int initial_seed)
        {
            int seed_parallel = initial_seed;
#pragma omp parallel for num_threads(N_OF_THREADS)
            for (int i = 0; i < n_evals; i++)
            {
                res[i] = FitnessFunction_real_func(net, seed_parallel + i, parameters->N_EVALS, parameters);
            }
            seed_parallel += n_evals;
            //PrintArray(res, n_evals);
        }

        __net_eval_decl void execute(class NEAT::Network **nets_, class NEAT::OrganismEvaluation *results, size_t nnets)
        {
            NEAT::CpuNetwork **nets = (NEAT::CpuNetwork **)nets_;
            double progress_print_decider = 0.0;
            double *f_values = new double[nnets];
            bool printed_bracket = false;
            RandomNumberGenerator rng;
            rng.seed();
            int initial_seed = rng.random_integer_fast(10000000, 20000000);
            cout << "Evaluating -> ";
            progress_bar bar = progress_bar(nnets);
            // evaluate the individuals
            #pragma omp parallel for num_threads(N_OF_THREADS)
            for (size_t inet = 0; inet < nnets; inet++)
            {
                bar.step();
                NEAT::CpuNetwork *net = nets[inet];
                NEAT::OrganismEvaluation eval;
                int seed = initial_seed;
                f_values[inet] = this->FitnessFunction(net, parameters->N_EVALS, seed);
                results[inet] = eval;

            }
            bar.end();


        }
    };
}

namespace NEAT
{

class real_funcEvaluator : public NetworkEvaluator
{
    NetworkExecutor<REAL_FUNC::Evaluator> *executor;
    REAL_FUNC::params *parameters;

public:
    real_funcEvaluator()
    {
        executor = NetworkExecutor<REAL_FUNC::Evaluator>::create();
        parameters = new REAL_FUNC::params();
    }

    ~real_funcEvaluator()
    {
        delete executor;
    }

    virtual void execute(class Network **nets_,
                         class OrganismEvaluation *results,
                         size_t nnets)
    {
        using namespace NEAT;
        env->pop_size = POPSIZE_NEAT;
        REAL_FUNC::Evaluator *ev = new REAL_FUNC::Evaluator();
        ev->parameters = this->parameters;
        ev->execute(nets_, results, nnets);
        delete ev;
    }

    virtual void run_given_conf_file(std::string conf_file_path)
    {

        global_timer.tic();

        INIReader reader(conf_file_path);

        if (reader.ParseError() != 0)
        {
            std::cout << "Can't load " << conf_file_path << "\n";
            exit(1);
        }

        std::string MODE = reader.Get("Global", "mode", "UNKNOWN");
        std::string prob_name = reader.Get("Global", "PROBLEM_NAME", "UNKNOWN");

        if (MODE == "train")
        {
            int rng_seed = reader.GetInteger("NEAT", "SEED", -1);
            env->pop_size = reader.GetInteger("NEAT", "POPSIZE", -1);
            int max_time = reader.GetInteger("NEAT", "MAX_TRAIN_TIME", -1);
            N_OF_THREADS = reader.GetInteger("NEAT", "THREADS", -1);
            parameters->N_EVALS = reader.GetInteger("NEAT", "N_EVALS", -1);
            parameters->N_REEVALS_TOP_5_PERCENT = reader.GetInteger("NEAT", "N_REEVALS_TOP_5_PERCENT", -1);
            string search_type = reader.Get("NEAT", "SEARCH_TYPE", "UNKOWN");
            parameters->PROBLEM_INDEX = reader.GetInteger("Controller", "PROBLEM_INDEX", -1);
            parameters->PROBLEM_DIMENSIONS = reader.GetInteger("Controller", "PROBLEM_DIMENSIONS", -1);
            parameters->MAX_EVALS_PSO = reader.GetInteger("Controller", "MAX_EVALS_PSO", -1);
            parameters->POPSIZE = reader.GetInteger("Controller", "POPSIZE", -1);
            parameters->TABU_LENGTH = reader.GetInteger("Controller", "TABU_LENGTH", -1);
            EXPERIMENT_FOLDER_NAME = "controllers_trained_with_F" + std::to_string(parameters->PROBLEM_INDEX);
            parameters->PRINT_POSITIONS = false;
            parameters->UPDATE_BK_EVERY_K_ITERATIONS = reader.GetInteger("NEAT", "UPDATE_BK_EVERY_K_ITERATIONS", -1);
            parameters->SAMPLE_SIZE_UPDATE_BK = reader.GetInteger("NEAT", "SAMPLE_SIZE_UPDATE_BK", -1);
            parameters->N_SAMPLES_UPDATE_BK = reader.GetInteger("NEAT", "N_SAMPLES_UPDATE_BK", -1);

            assert(parameters->N_SAMPLES_UPDATE_BK > 0);
            parameters->bk_f_average_sample = new double[parameters->N_SAMPLES_UPDATE_BK];
            for (int i = 0; i < parameters->N_SAMPLES_UPDATE_BK; i++)
            {
                parameters->bk_f_average_sample[i] = -DBL_MAX;
            }

            load_global_params(conf_file_path);

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
                cout << "Warning: a minimum of 7 cores (specified by the THREADS parameter) is recommended for this implementation of NEAT to function correctly." << endl;
            }

            if (env->pop_size < 500)
            {
                cout << "Warning: The population size of the controllers might be too low." << endl;
                cout << "The provided population size of the controllers is " << env->pop_size << ", a value of at least 500 is recommended." << endl;
                cout << endl
                     << endl;
            }

            MAX_TRAIN_TIME = max_time;

            if (env->search_type == GeneticSearchType::BLENDED)
            {
                env->mutate_delete_node_prob *= 0.1;
                env->mutate_delete_link_prob *= 0.1;
            }

            Experiment *exp = Experiment::get(prob_name.c_str());
            rng_t rng{rng_seed};
            global_timer.tic();
            exp->run(rng);

            delete[] parameters->bk_f_average_sample;

            exit(0);
        }
        else if (MODE == "test")
        {

            //const char * prob_name = "permu";
            //Experiment *exp = Experiment::get(prob_name);

            parameters->PROBLEM_INDEX = reader.GetInteger("Controller", "PROBLEM_INDEX", -1);
            parameters->PROBLEM_DIMENSIONS = reader.GetInteger("Controller", "PROBLEM_DIMENSIONS", -1);
            parameters->MAX_EVALS_PSO = reader.GetInteger("Controller", "MAX_EVALS_PSO", -1);
            parameters->POPSIZE = reader.GetInteger("Controller", "POPSIZE", -1);
            parameters->TABU_LENGTH = reader.GetInteger("Controller", "TABU_LENGTH", -1);
            parameters->CONTROLLER_PATH = reader.Get("TestSettings", "CONTROLLER_PATH", "UNKNOWN");
            parameters->N_REPS = reader.GetInteger("TestSettings", "N_REPS", -1);
            parameters->N_EVALS = reader.GetInteger("TestSettings", "N_EVALS", -1);
            N_OF_THREADS = reader.GetInteger("TestSettings", "THREADS", 1);
            N_OF_THREADS = min(N_OF_THREADS, parameters->N_EVALS);
            parameters->PRINT_POSITIONS = reader.GetBoolean("TestSettings", "PRINT_POSITIONS", false);


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
            RandomNumberGenerator *rng;
            rng = new RandomNumberGenerator();
            rng->seed(2);

            int initial_seed = rng->random_integer_uniform(1000, 10000);

            for (int j = 0; j < parameters->N_REPS; j++)
            {
#pragma omp parallel for num_threads(N_OF_THREADS)
                for (int i = 0; i < parameters->N_EVALS; i++)
                {
                    v_of_f_values[i] = FitnessFunction_real_func(&net, initial_seed + i, 1, parameters);
                }
                initial_seed += parameters->N_EVALS;
                double res = Average(v_of_f_values, parameters->N_EVALS);
                cout << res << ",";
            }
            delete[] v_of_f_values;

            cout << std::setprecision(15);
            cout << std::flush;
            cout << "|F" << parameters->PROBLEM_INDEX
                 << "|" << parameters->CONTROLLER_PATH
                 << "|dim: " << parameters->PROBLEM_DIMENSIONS
                 << "|" << parameters->MAX_EVALS_PSO << "|" << endl;

            // cout << res << std::endl;;
            cout << std::flush;

            exit(0);
        }
        else
        {
            cout << "invalid mode provided. Please, use the configuration file to specify either test or train.";
            exit(1);
        }
    }
};

class NetworkEvaluator *create_real_func_evaluator()
{
    return new real_funcEvaluator();
};

} // namespace NEAT
