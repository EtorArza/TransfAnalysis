#include "std.h" // Must be included first. Precompiled header with standard library includes.
#include "experiment.h"
#include "util.h"
#include "network.h"
#include "networkexecutor.h"
#include "neat.h"
#include "PERMU_MULTI_params.h"
#include "constants.h"

using namespace NEAT;
using namespace std;

//------------------------------
//---
//--- CLASS Experiment
//---
//------------------------------
map<string, Experiment *> *Experiment::experiments = nullptr;

Experiment *Experiment::get(const char *name) {
    if(!experiments) {
        experiments = new map<string, Experiment*>();
    }
    auto it = experiments->find(name);
    return it == experiments->end() ? nullptr : it->second;
}

vector<string> Experiment::get_names() {
    vector<string> result;
    if(experiments) {
        for(auto &kv: *experiments) {
            result.push_back(kv.first);
        }
    }
    return result;
}

Experiment::Experiment(const char *name) {
    this->name = name;
    if(get(name) != nullptr) {
        trap("Experiment already registered: " << name);
    }
    experiments->insert(make_pair(name, this));
}

Experiment::~Experiment() {
    experiments->erase(name);
    if(experiments->size() == 0) {
        delete experiments;
        experiments = nullptr;
    }
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

void convert_f_values_to_negative_inverse_ranks_squared(vector<int> surviving_candidates, double **f_values, double **ranks, int current_n_of_evals)
{
    convert_f_values_to_ranks(surviving_candidates, f_values, ranks, current_n_of_evals);
    for (int i = 0; i < surviving_candidates.size(); i++)
    {
        for (int j = 0; j < current_n_of_evals; j++)
        {
            ranks[surviving_candidates[i]][j] = - pow(surviving_candidates.size() - ranks[surviving_candidates[i]][j] + 1, 2);
        }
    }
}

bool check_if_all_ranks_are_the_same(vector<int> surviving_candidates, double **ranks, int current_n_of_evals)
{
    double **all_values;
    all_values = new double *[surviving_candidates.size()];

    for (int i = 0; i < surviving_candidates.size(); i++)
    {
        all_values[i] = ranks[surviving_candidates[i]];
    }
    bool res = are_all_values_the_same_in_matrix(all_values, surviving_candidates.size(), current_n_of_evals);
    delete[] all_values;
    return res;
}

void execute_multi(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets, int n_instances, FF_type FitnessFunction, NEAT::CpuNetwork *&best_network, base_params *parameters)
{

        #ifdef HIPATIA
        const bool flushOut = false;
        #else
        const bool flushOut = true;
        #endif

        if (n_instances == 0)
        {
            cout << "n_instances = 0 in execute_multi()" << endl;
            exit(1);
        }



        using namespace PERMU;
        NEAT::CpuNetwork **nets = (NEAT::CpuNetwork **)nets_;
        double **f_values;
        double **f_value_ranks;

        int ALPHA_INDEX = 2;
        int target_n_controllers_left = 1;
        int n_evals_each_it = 32;
        int row_length =  MAX_EVALS_PER_CONTROLLER + 2*n_evals_each_it;

        zero_initialize_matrix(f_values, nnets + 1, row_length);
        zero_initialize_matrix(f_value_ranks, nnets + 1, row_length);
        int initial_seed;

        double *tmp_order = new double[nnets+1];
        // evaluate the individuals

        if (best_network == nullptr)
        {
            best_network = new NEAT::CpuNetwork(*nets[0]);
        }


        int current_n_of_evals = 0;

        vector<int> surviving_candidates;

        for (int inet = 0; inet < nnets; inet++)
        {
            surviving_candidates.push_back(inet);
        }


        initial_seed = global_rng.random_integer_uniform(INT_MAX / 10);



        while (surviving_candidates.size() > target_n_controllers_left && MAX_EVALS_PER_CONTROLLER > current_n_of_evals)
        {
            #ifndef HIPATIA
            cout << "Evaluating -> ";
            #endif
            //cout << "\n" << "inet" << "," << "f_value_sample_index" << "," << "instance_index" << "," << "seed" << "\n";

            int n_surviving_candidates = surviving_candidates.size();
            
            #ifndef HIPATIA
            progress_bar bar(n_surviving_candidates* n_evals_each_it, flushOut);
            #endif

            #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS) schedule(dynamic,1)
            for (int i = 0; i < n_surviving_candidates * n_evals_each_it; i++)
            {
                int inet = surviving_candidates[i % n_surviving_candidates];
                int f_value_sample_index = i / n_surviving_candidates + current_n_of_evals;
                int instance_index =  f_value_sample_index % n_instances;

                NEAT::CpuNetwork *net = nets[inet];
                int seed = initial_seed + f_value_sample_index;

                f_values[inet][f_value_sample_index] = FitnessFunction(net, seed, instance_index, parameters);
                #ifndef HIPATIA
                    bar.step();
                #endif
            }
            #ifndef HIPATIA
                bar.end();
                cout << ", ";
            #endif
            current_n_of_evals += n_evals_each_it;

            convert_f_values_to_negative_inverse_ranks_squared(surviving_candidates, f_values, f_value_ranks, current_n_of_evals);


            for (auto &&inet : surviving_candidates)
            {
                tmp_order[inet] = Average(f_value_ranks[inet], current_n_of_evals) - (double)surviving_candidates.size() * 10000000.0;
            }

            bool are_all_ranks_the_same = check_if_all_ranks_are_the_same(surviving_candidates, f_value_ranks, current_n_of_evals);

            if (!are_all_ranks_the_same)
            {
                F_race_iteration(f_value_ranks, surviving_candidates, current_n_of_evals, ALPHA_INDEX);
            }else
            {
                #ifndef HIPATIA
                cout << "The controllers behave the same.";
                #endif
            }
            #ifndef HIPATIA
            cout << ", perc_discarded: " << (double)(nnets - surviving_candidates.size()) / (double)(nnets);
            cout << ", " << surviving_candidates.size() << " left.";
            cout << "\n";
            #endif
            if (are_all_ranks_the_same)
            {
                break;
            }
        }



        for (auto &&inet : surviving_candidates)
        {
            tmp_order[inet] = Average(f_value_ranks[inet], current_n_of_evals) - (double)surviving_candidates.size() * 10000000.0;
        }

        int best_current_iteration_index = argmax(tmp_order, (int)nnets);



        // update best known of last iteration and this iteration
        initial_seed = global_rng.random_integer_uniform(INT_MAX / 10);
        surviving_candidates.clear();
        surviving_candidates.push_back(best_current_iteration_index);
        surviving_candidates.push_back(nnets);

        #ifndef HIPATIA
        cout << "Reevaluating best -> ";
        progress_bar bar(MAX_EVALS_PER_CONTROLLER, flushOut);
        #endif

        current_n_of_evals = 0;
        bool test_result = true;

        while (test_result && current_n_of_evals < MAX_EVALS_PER_CONTROLLER)
        {
            #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS) schedule(dynamic,1)
            for (int i = 0; i < MAX_EVALS_PER_CONTROLLER; i++)
            {

                #ifndef HIPATIA
                    bar.step();
                #endif
                int instance_index =  i % n_instances;
                int f_value_sample_index = i + current_n_of_evals;
                int seed = f_value_sample_index + initial_seed;


                NEAT::CpuNetwork *net = nets[best_current_iteration_index];
                f_values[best_current_iteration_index][f_value_sample_index] = FitnessFunction(net, seed, instance_index, parameters);

                net = best_network;
                f_values[nnets][f_value_sample_index] = FitnessFunction(net, seed, instance_index, parameters);
            }
            current_n_of_evals += MAX_EVALS_PER_CONTROLLER;
            convert_f_values_to_negative_inverse_ranks_squared(surviving_candidates, f_values, f_value_ranks, current_n_of_evals);
            if (check_if_all_ranks_are_the_same(surviving_candidates, f_value_ranks, current_n_of_evals))
            {
                test_result = false;
                #ifndef HIPATIA
                    cout << "The controllers behave the same.";
                #endif
                break;
            }
            // cout << "best this gen ->";
            // PrintArray(f_value_ranks[best_current_iteration_index], current_n_of_evals);
            // cout << "best last gen ->";
            // PrintArray(f_value_ranks[nnets], current_n_of_evals);
            test_result = is_A_larger_than_B_sign_test(f_value_ranks[best_current_iteration_index], f_value_ranks[nnets], current_n_of_evals, ALPHA_INDEX);
            //cout << "test_result: " << test_result << "\n";
        }
        #ifndef HIPATIA
        bar.end();
        cout << "\n";
        #endif


        convert_f_values_to_negative_inverse_ranks_squared(surviving_candidates, f_values, f_value_ranks, current_n_of_evals);
        double avg_perf_best_last = Average(f_value_ranks[nnets], current_n_of_evals) - 1;
        double avg_perf_best_current = Average(f_value_ranks[best_current_iteration_index], current_n_of_evals) - 1;
        tmp_order[best_current_iteration_index] = 10e20;
        

        parameters->neat_params->BEST_FITNESS_TRAIN = 1.0;

        cout << "(best this gen, best last gen) -> (" << avg_perf_best_current << ", " << avg_perf_best_last << ")" << ", higher is better";

        if (test_result)
        {
            parameters->neat_params->N_TIMES_BEST_FITNESS_IMPROVED_TRAIN++;
            parameters->neat_params->N_ITERATIONS_WITHOUT_FITNESS_IMPROVED = 0;

            cout << ", best replaced";
            parameters->neat_params->BEST_FITNESS_TRAIN = 1.0;
            delete best_network;
            best_network = new NEAT::CpuNetwork(*nets[best_current_iteration_index]);
        }
        else
        {
            cout << ", " 
            << parameters->neat_params->N_ITERATIONS_WITHOUT_FITNESS_IMPROVED 
            << " it since last best found";
            parameters->neat_params->N_ITERATIONS_WITHOUT_FITNESS_IMPROVED++;
        }
        

        cout << endl;



        compute_order_from_double_to_double(tmp_order, nnets, tmp_order, false, true);



        multiply_array_with_value(tmp_order, 1.0 / (double)(nnets - 1), (int)nnets);
        double best_rank = tmp_order[best_current_iteration_index];

        // if there are ties in the best score, without this correction, there might be 'fake' new best scores
        for (int i = 0; i < nnets; i++)
        {
            if (tmp_order[i] == best_rank)
            {
                tmp_order[i] = 1.0;
            }
        }        
        multiply_array_with_value(tmp_order, 1.0 + ((double)parameters->neat_params->N_TIMES_BEST_FITNESS_IMPROVED_TRAIN / 1000.0), (int)nnets);



        // save scaled fitness
        for (size_t inet = 0; inet < nnets; inet++)
        {
            NEAT::OrganismEvaluation eval;
            results[inet] = eval;
            results[inet].fitness = tmp_order[inet];
            results[inet].error = 2 - tmp_order[inet];
        }

        delete_matrix(f_values, nnets + 1);
        delete_matrix(f_value_ranks, nnets + 1);

        delete[] tmp_order;
}