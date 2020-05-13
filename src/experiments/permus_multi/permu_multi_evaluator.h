#pragma once

#include "permuevaluator.h"
#include "PERMU_MULTI_params.h"


namespace PERMU_MULTI{
    struct Evaluator;
}


namespace NEAT
{



class Permu_multiEvaluator : public PermuEvaluator
{   


public:

    NetworkExecutor<PERMU_MULTI::Evaluator> *executor_multi;
    PERMU_MULTI::params_multi *parameters_multi;

    Permu_multiEvaluator();
    ~Permu_multiEvaluator();

    void read_conf_file(std::string conf_file_path);
    void run_given_conf_file(std::string conf_file_path);
    void execute(class NEAT::Network **nets_, class NEAT::OrganismEvaluation *results, size_t nnets);
};

class NetworkEvaluator *create_permu_evaluator();

} // namespace NEAT
