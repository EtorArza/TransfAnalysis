#pragma once
#include "network.h"
#include "networkexecutor.h"
#include "REAL_FUNC_params.h"
#include "evaluatorexperiment.h"

 
namespace REAL_FUNC
{

    enum sensor_t {
        ABSOLUTE_DIST_TO_AVERAGE = 0,
        ABSOLUTE_DIST_TO_BEST = 1,
        RELATIVE_DIST_TO_CLOSEST = 2,
        RELATIVE_DIST_TO_AVERAGE = 3,
        RELATIVE_DIST_TO_BEST = 4,
        RELATIVE_TIME = 5,
        RELATIVE_SCORE = 6,
        INDIVIDUAL_BEST_WAS_IMPROVED = 7,
        GLOBAL_BEST_WAS_IMPROVED = 8,
        RANDOM_NUMBER = 9,
        __sensor_N = 10,
    };

    enum output_t {
        MOMENTUM = 0,
        G_BEST = 1,
        L_BEST = 2,
        AVERAGE = 3,
        RANDOM = 4,
        __output_N = 5,
        __output_N_reduced_model = 3,
    };

}

namespace REAL_FUNC
{
    struct Evaluator;
}

class MultidimBenchmarkFF;
MultidimBenchmarkFF *load_problem(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int seed_randomly_generated_instance=2);

namespace NEAT
{

class real_funcEvaluator : public NetworkEvaluator
{   


public:

    REAL_FUNC::params *parameters;
    NetworkExecutor<REAL_FUNC::Evaluator> *executor;
    CpuNetwork* best_network = nullptr;
    int iteration_number;
    double *average_f_values_obtained_by_bk;

    real_funcEvaluator();
    ~real_funcEvaluator();

    void read_conf_file(std::string conf_file_path);
    void run_given_conf_file(std::string conf_file_path);
    void execute(class NEAT::Network **nets_, class NEAT::OrganismEvaluation *results, size_t nnets);
};

class NetworkEvaluator *create_real_func_evaluator();

} // namespace NEAT
