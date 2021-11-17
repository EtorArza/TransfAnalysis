#pragma once
#include "Parameters.h"
namespace NEAT {

    class Network;
    class OrganismEvaluation;
    class CpuNetwork;

    class Experiment {
    public:
        neat_parameters *neat_params;
        static std::vector<std::string> get_names();
        static Experiment *get(const char *name);

    private:
        static std::map<std::string, Experiment*> *experiments;

    public:
        virtual ~Experiment();

        virtual void run(class rng_t &rng) = 0;
        virtual void run_given_conf_file(std::string conf_file_path) = 0;


    protected:
        Experiment(const char *name);

    private:
        Experiment() {}

        const char *name;
    };
}




typedef double (*FF_type)(NEAT::CpuNetwork *net, uint32_t initial_seed, int instance_index, base_params *parameters);  // Declare typedef

void convert_f_values_to_ranks(vector<int> surviving_candidates, double **f_values, double **ranks, int current_n_of_evals);

void  execute_multi(class NEAT::Network **nets_, NEAT::OrganismEvaluation *results, size_t nnets, int n_instances, FF_type FitnessFunction, NEAT::CpuNetwork *&best_network, base_params *parameters);
