#include "std.h" // Must be included first. Precompiled header with standard library includes.
#include "experiment.h"
#include "util.h"
#include "network.h"
#include "networkexecutor.h"
#include "neat.h"
#include "PERMU_MULTI_params.h"

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




void  execute_multi(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets, int n_instances, FF_type FitnessFunction, NEAT::CpuNetwork *&best_network, base_params *parameters)
{
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
            #pragma omp parallel for num_threads(parameters->neat_params->N_OF_THREADS) schedule(dynamic,1)
            for (int i = 0; i < n_surviving_candidates * n_evals_each_it; i++)
            {
                int inet = surviving_candidates[i % n_surviving_candidates];
                int f_value_sample_index = i / n_surviving_candidates;
                int instance_index =  f_value_sample_index % n_instances;

                NEAT::CpuNetwork *net = nets[inet];
                int seed = initial_seed + i / n_surviving_candidates;



                f_values[inet][f_value_sample_index] = FitnessFunction(net, seed, instance_index, parameters);
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
            f_values[nnets][i] = FitnessFunction(best_network, initial_seed + i, i % n_instances, parameters);
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