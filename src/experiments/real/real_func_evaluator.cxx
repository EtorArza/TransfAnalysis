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

    for (int i = 0; i < pop->popsize; i++)
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
                net->clear_noninput();
                for (int sns_idx = 0; sns_idx < REAL_FUNC::__sensor_N; sns_idx++)
                {
                    net->load_sensor(sns_idx, pop->get_neat_input_individual_i(i)[sns_idx]);
                }
                net->activate();
                pop->apply_neat_output_to_individual_i(net->get_outputs(), i);
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
    return FitnessFunction_real_func(net_original, parameters->PROBLEM_INDEX, parameters->PROBLEM_DIM, n_evals, seed, parameters);
}

double FitnessFunction_real_func(NEAT::CpuNetwork *net_original, int seed, REAL_FUNC::params *parameters)
{
    return FitnessFunction_real_func(net_original, parameters->PROBLEM_INDEX, parameters->PROBLEM_DIM, 1, seed, parameters);
}

void convert_f_values_to_ranks(vector<int> surviving_candidates, double **f_values, double **ranks, int current_n_of_evals)
{
    // replace f values with ranks
    static vector<vector<double>> ranks_vec;
    double *f_values_surviving_candidates[surviving_candidates.size()]; 
    ranks_vec.resize(surviving_candidates.size());
    for (int i = 0; i < surviving_candidates.size(); i++)
    {
        f_values_surviving_candidates[i] = f_values[surviving_candidates[i]];
        ranks_vec[i].resize(current_n_of_evals);
    }

    get_ranks_from_f_values(ranks_vec, f_values_surviving_candidates, surviving_candidates.size(), current_n_of_evals);
    for (int i = 0; i < surviving_candidates.size(); i++)
    {
        for (int j = 0; j < current_n_of_evals; j++)
        {
            ranks[surviving_candidates[i]][j] = ranks_vec[i][j];
        }
    }
}



namespace REAL_FUNC
{


struct Evaluator
{

    REAL_FUNC::params *parameters;
    NEAT::CpuNetwork *best_network;
    int iteration_number = 0;

    __net_eval_decl Evaluator(){};

    // fitness function in sequential order
    __net_eval_decl double FitnessFunction(NEAT::CpuNetwork *net, int initial_seed, int instance_index)
    {
        int seed_seq = initial_seed;
        REAL_FUNC::params tmp_params = *parameters;
        tmp_params.PROBLEM_INDEX = (*parameters->PROBLEM_INDEX_LIST)[instance_index];
        tmp_params.PROBLEM_DIM = (*parameters->PROBLEM_DIM_LIST)[instance_index];
        double res = FitnessFunction_real_func(net, seed_seq, &tmp_params);
        return res;
    }


    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets)
    {


        

        int n_instances = (*this->parameters->PROBLEM_INDEX_LIST).size();
        int n_reps_all_instances = EVAL_MIN_STEP / n_instances;
        int n_evals_each_it = n_reps_all_instances * n_instances;

        if (n_instances == 0)
        {
            cout << "n_instances = 0 in permu_multi_evaluator.cxx" << endl;
            exit(1);
        }



        using namespace PERMU;
        NEAT::CpuNetwork **nets = (NEAT::CpuNetwork **)nets_;
        double **f_values = new double *[nnets+1];
        double **f_value_ranks = new double *[nnets+1];

        RandomNumberGenerator rng;

        for (size_t i = 0; i < nnets+1; i++)
        {
            f_values[i] = new double[EVALS_TO_SELECT_BEST_CONTROLLER_IN_LAST_IT + EVAL_MIN_STEP + n_evals_each_it];
            f_value_ranks[i] = new double[EVALS_TO_SELECT_BEST_CONTROLLER_IN_LAST_IT + EVAL_MIN_STEP + n_evals_each_it];
        }

        double *tmp_order = new double[nnets+1];
        // evaluate the individuals

        if (best_network == nullptr)
        {
            best_network = new NEAT::CpuNetwork(*nets[0]);
        }


        int current_n_of_evals = 0;
        int target_n_controllers_left;
        int max_evals_per_controller;
        int ALPHA_INDEX;

        vector<int> surviving_candidates;

        for (int inet = 0; inet < nnets; inet++)
        {
            surviving_candidates.push_back(inet);
        }


        if (!parameters->neat_params->IS_LAST_ITERATION)
        {
            target_n_controllers_left = (int)((double)nnets * NEAT::env->survival_thresh);
            max_evals_per_controller = MAX_EVALS_PER_CONTROLLER_NEUROEVOLUTION;
            ALPHA_INDEX = 0;
        }
        else
        {
            target_n_controllers_left = 1;
            max_evals_per_controller = EVALS_TO_SELECT_BEST_CONTROLLER_IN_LAST_IT;
            ALPHA_INDEX = 1;
        }
        




        while (surviving_candidates.size() > target_n_controllers_left && max_evals_per_controller > current_n_of_evals)
        {
            int initial_seed = rng.random_integer_uniform(INT_MAX);
            cout << "Evaluating -> " << std::flush;
            //cout << endl << "inet" << "," << "f_value_sample_index" << "," << "instance_index" << "," << "seed" << endl;

            int n_surviving_candidates = surviving_candidates.size();
            progress_bar bar(n_surviving_candidates* n_evals_each_it);
            #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS)
            for (int i = 0; i < n_surviving_candidates * n_evals_each_it; i++)
            {
                int inet = surviving_candidates[i % n_surviving_candidates];
                int f_value_sample_index = i / n_surviving_candidates;
                int instance_index =  f_value_sample_index % n_instances;

                NEAT::CpuNetwork *net = nets[inet];
                int seed = initial_seed + i / n_surviving_candidates;

                //cout << inet << "," << f_value_sample_index << "," << instance_index << "," << seed << endl;


                f_values[inet][f_value_sample_index] = this->FitnessFunction(net, seed, instance_index);
                bar.step();
            }

            bar.end();
            cout << ", ";
            current_n_of_evals += n_evals_each_it;

            convert_f_values_to_ranks(surviving_candidates, f_values, f_value_ranks, current_n_of_evals);


            for (auto &&inet : surviving_candidates)
            {
                tmp_order[inet] = Average(f_value_ranks[inet], current_n_of_evals) - (double)surviving_candidates.size() * 10000000.0;
            }



            F_race_iteration(f_value_ranks, surviving_candidates, current_n_of_evals, ALPHA_INDEX);


            cout << ", perc_discarded: " << (double)(nnets - surviving_candidates.size()) / (double)(nnets);
            cout << ", " << surviving_candidates.size() << " left.";
            cout << endl;
        }


        // update best known of last iteration
        int initial_seed = rng.random_integer_uniform(INT_MAX);
        #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS)
        for (int i = 0; i < current_n_of_evals; i++)
        {
            f_values[nnets][i] = this->FitnessFunction(best_network, initial_seed + i, i % n_instances);
        }
        surviving_candidates.push_back((int) nnets);
        convert_f_values_to_ranks(surviving_candidates, f_values, f_value_ranks, current_n_of_evals);
        double avg_perf_best = Average(f_value_ranks[nnets], current_n_of_evals);
        parameters->neat_params->BEST_FITNESS_TRAIN = (avg_perf_best + parameters->neat_params->BEST_FITNESS_TRAIN) / 2;

        for (auto &&inet : surviving_candidates)
        {
            tmp_order[inet] = Average(f_value_ranks[inet], current_n_of_evals) - (double)surviving_candidates.size() * 10000000.0;
        }

        double best_f_gen = Average(f_value_ranks[argmax(tmp_order, (int)nnets)], current_n_of_evals);
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
            PrintMatrix(f_values, nnets, n_evals_each_it);

            cout << "franks_matrix (it is normal that ranks are repeated, since rows are updated in every iteration, with only the surviving_candidates being updated): " << std::flush;
            PrintMatrix(f_value_ranks, nnets, n_evals_each_it);

            for (int i = 0; i < nnets; i++)
            {
                cout << "|" << " i = " << i << " " << Average(f_values[i], n_evals_each_it) << endl; 
            }
            

            cout << endl;
            cout << "tmp_order: " << std::flush;
            PrintArray(tmp_order, nnets);

            cout << endl;

            double avg_perf_best = 0;


            cout << "BEST_FITNESS_DEBUG_LAST_IT, (reeval200, selection_best) ->" << 0 << ", " << Average(f_values[argmax(tmp_order, (int)nnets)], EVALS_TO_SELECT_BEST_CONTROLLER_IN_LAST_IT) << endl;
            
            cout << endl;
        }




        // save scaled fitness
        for (size_t inet = 0; inet < nnets; inet++)
        {
            NEAT::OrganismEvaluation eval;
            results[inet] = eval;
            results[inet].fitness = tmp_order[inet];
            results[inet].error = 2 - tmp_order[inet];
        }

        for (size_t i = 0; i < nnets+1; i++)
        {
            delete[] f_values[i];
            delete[] f_value_ranks[i];
        }
        delete[] f_values;
        delete[] f_value_ranks;

        delete[] tmp_order;
    }
};

} // namespace REAL_FUNC

namespace NEAT
{
    real_funcEvaluator::real_funcEvaluator()
    {
        executor = NetworkExecutor<REAL_FUNC::Evaluator>::create();
        parameters = new REAL_FUNC::params();
        best_network = nullptr;
        iteration_number = 0;
    }

    real_funcEvaluator::~real_funcEvaluator()
    {
        delete executor;
    }

    void real_funcEvaluator::execute(class Network **nets_, class OrganismEvaluation *results, size_t nnets)
    {
        using namespace NEAT;
        env->pop_size = this->neat_params->POPSIZE_NEAT;
        REAL_FUNC::Evaluator *ev = new REAL_FUNC::Evaluator();
        this->parameters->neat_params = this->neat_params;
        ev->parameters = this->parameters;
        ev->iteration_number = this->iteration_number;
        ev->best_network = this->best_network;
        ev->execute(nets_, results, nnets);

        this->best_network = ev->best_network;
        this->iteration_number++;
        delete ev;
    }

    void real_funcEvaluator::read_conf_file(std::string conf_file_path)
    {
        using namespace std;
        using namespace NEAT;

        INIReader reader(conf_file_path);

        if (reader.ParseError() != 0)
        {
            std::cout << "Can't load " << conf_file_path << "\n";
            exit(1);
        }

        parameters->SEED = reader.GetInteger("Global", "SEED", -1);
        if (parameters->SEED == -1)
        {
            RandomNumberGenerator tmp_rng;
            parameters->SEED = tmp_rng.random_integer_uniform(INT_MAX);
        }
        parameters->prob_name = reader.Get("Global", "PROBLEM_NAME", "UNKNOWN");
        parameters->MODE = reader.Get("Global", "MODE", "UNKNOWN");
        parameters->SOLVER_POPSIZE = reader.GetInteger("Global", "SOLVER_POPSIZE", -1);


        if (parameters->MODE == "train")
        {

            string search_type = reader.Get("Global", "SEARCH_TYPE", "UNKOWN");

            std::string COMMA_SEPARATED_LIST;

            COMMA_SEPARATED_LIST = reader.Get("Global", "COMMA_SEPARATED_PROBLEM_INDEX_LIST", "UNKNOWN");
            parameters->PROBLEM_INDEX_LIST = new std::vector<int>(from_comma_sep_values_in_string_to_int_vector(COMMA_SEPARATED_LIST));

            COMMA_SEPARATED_LIST = reader.Get("Global", "COMMA_SEPARATED_PROBLEM_DIM_LIST", "UNKNOWN");
            parameters->PROBLEM_DIM_LIST = new std::vector<int>(from_comma_sep_values_in_string_to_int_vector(COMMA_SEPARATED_LIST));

            parameters->MAX_SOLVER_FE = reader.GetInteger("Global", "MAX_SOLVER_FE", -1);

            if (neat_params->EXPERIMENT_FOLDER_NAME == "UNKNOWN")
            {
                std::string tmp_str = "";
                for (size_t i = 0; i < parameters->PROBLEM_INDEX_LIST->size(); i++)
                {
                    tmp_str += "_" + to_string((*parameters->PROBLEM_INDEX_LIST)[i]);
                }

                neat_params->EXPERIMENT_FOLDER_NAME = "controllers_trained_with" + tmp_str;
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

            parameters->MAX_SOLVER_FE = reader.GetReal("Global", "MAX_SOLVER_FE", -1.0);
            parameters->CONTROLLER_PATH = reader.Get("Global", "CONTROLLER_PATH", "UNKNOWN");
            parameters->N_REPS = reader.GetInteger("Global", "N_REPS", -1);
            parameters->N_EVALS = reader.GetInteger("Global", "N_EVALS", -1);
            parameters->PROBLEM_INDEX = reader.GetInteger("Global", "PROBLEM_INDEX", -1);
            parameters->PROBLEM_DIM = reader.GetInteger("Global", "PROBLEM_DIM", -1);
            parameters->COMPUTE_RESPONSE = reader.GetBoolean("Global", "COMPUTE_RESPONSE", false);
            parameters->PRINT_POSITIONS = reader.GetBoolean("Global", "PRINT_POSITIONS", false);
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

    void real_funcEvaluator::run_given_conf_file(std::string conf_file_path)
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
                net.start_recording_response();
            }

            double *v_of_f_values = new double[parameters->N_EVALS];

            cout << std::setprecision(15);
            RandomNumberGenerator *rng;
            rng = new RandomNumberGenerator();
            rng->seed();
            int initial_seed = rng->random_integer_uniform(40000000, 50000000);
            delete rng;
            ostringstream result_string_stream;
            result_string_stream << std::setprecision(15);
            result_string_stream << std::flush;

            result_string_stream << "[[";
            for (int j = 0; j < parameters->N_REPS; j++)
            {
                    #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS)
                    for (int i = 0; i < parameters->N_EVALS; i++)
                    {
                        v_of_f_values[i] = FitnessFunction_real_func(&net, initial_seed + i, parameters);
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

                result_string_stream << "\"" << parameters->PROBLEM_INDEX << "\",\""
                                     << parameters->PROBLEM_DIM << "\","
                                     << parameters->CONTROLLER_PATH << "\",\""
                                     << parameters->N_EVALS
                                     << "]"
                                     << endl;

                if (parameters->COMPUTE_RESPONSE)
                {
                    double *res = new double[REAL_FUNC::__output_N];
                    net.return_average_response_and_stop_recording(res);
                    append_line_to_file(
                        "responses.txt",
                        "['" + parameters->CONTROLLER_PATH + "', '" +
                            to_string(parameters->PROBLEM_INDEX) + "', '" +
                            to_string(parameters->PROBLEM_DIM) + "', " +
                            array_to_python_list_string(res, REAL_FUNC::__output_N) + "]\n");
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
                cout << "invalid mode provided. Please, use the configuration file to specify either test or train.";
                exit(1);
            }
        }

    class NetworkEvaluator *create_real_func_evaluator()
    {
        return new real_funcEvaluator();
    };

} // namespace NEAT
