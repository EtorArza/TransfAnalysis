#pragma once
#include "network.h"
#include "networkexecutor.h"
#include "REAL_FUNC_params.h"
#include "evaluatorexperiment.h"

 
namespace REAL_FUNC
{

    enum sensor_t {
        DIST_TO_CLOSEST = 0,
        DIST_TO_AVERAGE = 1,
        RELATIVE_DIST_TO_BEST = 2,
        RELATIVE_TIME = 3,
        RELATIVE_SCORE = 4,
        INDIVIDUAL_BEST_WAS_IMPROVED = 5,
        GLOBAL_BEST_WAS_IMPROVED = 6,
        RANDOM_NUMBER = 7,
        __sensor_N = 8,
    };

    enum output_t {
        MOMENTUM = 0,
        G_BEST = 1,
        L_BEST = 2,
        __output_N = 3,
    };

}

namespace REAL_FUNC
{
    struct Evaluator;
}


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
