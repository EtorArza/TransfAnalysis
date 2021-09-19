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
#define CURRICULUM_LEARNING


double FitnessFunction_real_func(class NEAT::CpuNetwork *net_original, int problem_index, int dim, int n_evals, int seed, REAL_FUNC::params *parameters)
{

    double *v_of_fitness;
    MultidimBenchmarkFF *problem;
    problem = load_problem(problem_index, dim, parameters->X_LOWER_LIM, parameters->X_UPPER_LIM, seed, true);
    
     

    CPopulation *pop;

    NEAT::CpuNetwork tmp_net = *net_original;
    NEAT::CpuNetwork *net = &tmp_net;


    pop = new CPopulation(problem, parameters);
    problem->load_rng(pop->rng);
    pop->rng->seed(seed);


    // Detailed response initialize ///////////////////////////////
    double*** detailed_response;
    const int N_OF_TIME_POSITIONS = 20;
    bool COMPUTE_RESPONSE = parameters->COMPUTE_RESPONSE;
    if (COMPUTE_RESPONSE)
    {

        detailed_response = new double **[2];
        zero_initialize_matrix(detailed_response[0],pop->popsize,REAL_FUNC::__output_N);
        zero_initialize_matrix(detailed_response[1],N_OF_TIME_POSITIONS,REAL_FUNC::__output_N);
    }



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
        int F_evals = -1;

        while (!pop->terminated)
        {

            if (parameters->PRINT_POSITIONS)
            {
                pop->print_positions("positions.txt");
            }

            for (int i = 0; i < pop->popsize; i++)
            {
                F_evals ++;
                net->clear_noninput();
                for (int sns_idx = 0; sns_idx < REAL_FUNC::__sensor_N; sns_idx++)
                {
                    net->load_sensor(sns_idx, pop->get_neat_input_individual_i(i)[sns_idx]);
                }
                net->activate();
                pop->apply_neat_output_to_individual_i(net->get_outputs(), i);
                if (COMPUTE_RESPONSE)
                {
                    sum_arrays(detailed_response[0][i], detailed_response[0][i], net->get_outputs(), REAL_FUNC::__output_N);
                    sum_arrays(detailed_response[1][N_OF_TIME_POSITIONS * F_evals / parameters->MAX_SOLVER_FE], detailed_response[1][N_OF_TIME_POSITIONS * F_evals / parameters->MAX_SOLVER_FE], net->get_outputs(), REAL_FUNC::__output_N);
                }
                
                
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

    if (COMPUTE_RESPONSE)
    {
        for (int i = 0; i < pop->popsize; i++)
        {
            multiply_array_with_value(detailed_response[0][i], (double) pop->popsize / (double) parameters->MAX_SOLVER_FE / (double) n_evals, REAL_FUNC::__output_N);
        }

        for (int i = 0; i < N_OF_TIME_POSITIONS; i++)
        {
            multiply_array_with_value(detailed_response[1][i], (double) N_OF_TIME_POSITIONS / (double) parameters->MAX_SOLVER_FE / (double) n_evals, REAL_FUNC::__output_N);
        }

        stringstream result_string_stream;
        cout << std::setprecision(6) << std::flush;
        result_string_stream << std::setprecision(6) << std::flush;

        PrintMatrix(detailed_response[0],pop->popsize,REAL_FUNC::__output_N, result_string_stream);
        PrintMatrix(detailed_response[1],N_OF_TIME_POSITIONS,REAL_FUNC::__output_N, result_string_stream);


        string result_string = result_string_stream.str();



        append_line_to_file("detailed_response.txt", result_string);



        delete_matrix(detailed_response[0], pop->popsize);
        delete_matrix(detailed_response[1], N_OF_TIME_POSITIONS);
        delete[] detailed_response;
    }

    delete[] v_of_fitness;
    delete pop;
    delete problem;
    
    pop = NULL;
    v_of_fitness = NULL;
    problem = NULL;
    net = NULL;
    return res;
}



namespace REAL_FUNC
{


// fitness function in sequential order
double FitnessFunction(NEAT::CpuNetwork *net, int seed, int instance_index, base_params *parameters)
{

    REAL_FUNC::params tmp_params = *static_cast<REAL_FUNC::params*>(parameters);
    #ifdef CURRICULUM_LEARNING
        const double min_progress = 0.001;
        #ifdef HIPATIA
            double progress = (double) get_runtime_hipatia() / (double) parameters->neat_params->MAX_TRAIN_TIME; 
        #else
            double progress = (double) parameters->neat_params->global_timer.toc() / (double) parameters->neat_params->MAX_TRAIN_TIME;
        #endif
        double max_evals = tmp_params.MAX_SOLVER_FE * (min(pow(progress, 4.0),1.0) + min_progress)/(1+min_progress); 
        tmp_params.MAX_SOLVER_FE = max((int) max_evals, tmp_params.SOLVER_POPSIZE * 10);
        static int counter = 0;
        counter++;
        if (counter % 10000 == 0)
        {
            cout << "\n[N evals: " << tmp_params.MAX_SOLVER_FE << "]" <<endl;
        }
    #endif 
    tmp_params.PROBLEM_INDEX = (*tmp_params.PROBLEM_INDEX_LIST)[instance_index];
    tmp_params.PROBLEM_DIM = (*tmp_params.PROBLEM_DIM_LIST)[instance_index];
    tmp_params.X_LOWER_LIM = (*tmp_params.X_LOWER_LIST)[instance_index];
    tmp_params.X_UPPER_LIM = (*tmp_params.X_UPPER_LIST)[instance_index];
    double res = FitnessFunction_real_func(net, tmp_params.PROBLEM_INDEX, tmp_params.PROBLEM_DIM, 1, seed, &tmp_params);
    return res;
}


struct Evaluator
{

    REAL_FUNC::params *parameters;
    NEAT::CpuNetwork *best_network;
    int iteration_number = 0;

    __net_eval_decl Evaluator(){};




    // compute the fitness value of all networks at training time.
    __net_eval_decl void execute(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets)
    {
        int n_instances = (*this->parameters->PROBLEM_INDEX_LIST).size();
        execute_multi(nets_, results, nnets, n_instances, FitnessFunction, best_network, parameters);
        // if ((*parameters->PROBLEM_INDEX_LIST)[0] == 10)
        // {
        //     double res;
        //     static int initial_seed = 6284314136;
        //     res = 0;
        //     int N = 7;
        //     #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS) schedule(dynamic,1)
        //     for (int i = 0; i < N; i++)
        //     {
        //         //int seed = initial_seed + i;
        //         res += FitnessFunction(best_network, initial_seed, 0, parameters);
        //     }
        //     res /= (double) N;
        //     cout << endl;
        //     cout << "best_fitness: " << res << endl; 
        // }

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

        parameters->prob_name = reader.Get("Global", "PROBLEM_NAME", "UNKNOWN");
        parameters->MODE = reader.Get("Global", "MODE", "UNKNOWN");
        parameters->SOLVER_POPSIZE = reader.GetInteger("Global", "SOLVER_POPSIZE", -1);
        parameters->MAX_SOLVER_FE = reader.GetInteger("Global", "MAX_SOLVER_FE", -1);
        parameters->FULL_MODEL = reader.GetBoolean("Global", "FULL_MODEL", false);

        if (parameters->MODE == "train")
        {

            string search_type = reader.Get("Global", "SEARCH_TYPE", "UNKOWN");

            std::string COMMA_SEPARATED_LIST;

            COMMA_SEPARATED_LIST = reader.Get("Global", "COMMA_SEPARATED_PROBLEM_INDEX_LIST", "UNKNOWN");
            parameters->PROBLEM_INDEX_LIST = new std::vector<int>(from_comma_sep_values_in_string_to_int_vector(COMMA_SEPARATED_LIST));

            COMMA_SEPARATED_LIST = reader.Get("Global", "COMMA_SEPARATED_PROBLEM_DIM_LIST", "UNKNOWN");
            parameters->PROBLEM_DIM_LIST = new std::vector<int>(from_comma_sep_values_in_string_to_int_vector(COMMA_SEPARATED_LIST));

            COMMA_SEPARATED_LIST = reader.Get("Global", "COMMA_SEPARATED_X_LOWER_LIST", "UNKNOWN");
            parameters->X_LOWER_LIST = new std::vector<int>(from_comma_sep_values_in_string_to_int_vector(COMMA_SEPARATED_LIST));

            COMMA_SEPARATED_LIST = reader.Get("Global", "COMMA_SEPARATED_X_UPPER_LIST", "UNKNOWN");
            parameters->X_UPPER_LIST = new std::vector<int>(from_comma_sep_values_in_string_to_int_vector(COMMA_SEPARATED_LIST));




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

            parameters->CONTROLLER_PATH = reader.Get("Global", "CONTROLLER_PATH", "UNKNOWN");
            parameters->N_REPS = reader.GetInteger("Global", "N_REPS", -1);
            parameters->N_EVALS = reader.GetInteger("Global", "N_EVALS", -1);
            parameters->PROBLEM_INDEX = reader.GetInteger("Global", "PROBLEM_INDEX", -1);
            parameters->PROBLEM_DIM = reader.GetInteger("Global", "PROBLEM_DIM", -1);
            parameters->COMPUTE_RESPONSE = reader.GetBoolean("Global", "COMPUTE_RESPONSE", false);
            parameters->PRINT_POSITIONS = reader.GetBoolean("Global", "PRINT_POSITIONS", false);
            neat_params->N_OF_THREADS = std::min(neat_params->N_OF_THREADS, parameters->N_EVALS);

            parameters->X_LOWER_LIM = reader.GetReal("Global", "X_LOWER_LIM", -1.0);
            parameters->X_UPPER_LIM = reader.GetReal("Global", "X_UPPER_LIM", -1.0);


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
            rng_t rng{parameters->neat_params->SEED};
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


            int initial_seed = global_rng.random_integer_uniform(INT_MAX / 10);

            

            ostringstream result_string_stream;
            cout << std::setprecision(30) << std::flush;
            result_string_stream << std::setprecision(30) << std::flush;


            result_string_stream << "[[";
            for (int j = 0; j < parameters->N_REPS; j++)
            {
                double res;
                if (parameters->COMPUTE_RESPONSE)
                {
                    res = FitnessFunction_real_func(&net, parameters->PROBLEM_INDEX, parameters->PROBLEM_DIM, parameters->N_EVALS, initial_seed + parameters->N_EVALS*j, parameters);
                }
                else
                {
                    #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS)
                    for (int i = 0; i < parameters->N_EVALS; i++)
                    {
                        v_of_f_values[i] = FitnessFunction_real_func(&net, parameters->PROBLEM_INDEX, parameters->PROBLEM_DIM, 1, initial_seed + i + parameters->N_EVALS*j, parameters);
                    }
                    initial_seed += parameters->N_EVALS;
                    res = Average(v_of_f_values, parameters->N_EVALS);
                }

                result_string_stream << res;
                if (j < parameters->N_REPS - 1)
                {
                    result_string_stream << ",";
                }
            }
            result_string_stream << "]," << std::flush;
            delete[] v_of_f_values;

            result_string_stream << "" << parameters->PROBLEM_INDEX << ","
                                 << std::setprecision(8)
                                 << std::flush
                                 << "(" << parameters->X_LOWER_LIM << "," << parameters->X_UPPER_LIM << "),"
                                 << std::setprecision(30)
                                 << std::flush

                                 << parameters->PROBLEM_DIM << ",\""
                                 << parameters->CONTROLLER_PATH << "\","
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
