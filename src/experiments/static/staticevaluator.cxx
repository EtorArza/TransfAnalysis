#include "std.hxx"

#include "network.h"
#include "networkexecutor.h"
#include "staticevaluator.h"

#include <assert.h>
#include <float.h>

using namespace NEAT;
using namespace std;



//---
//--- CLASS Evaluator
//---
//--- Manages a Network's sensors and evaluates a Network's output.
//---
struct Evaluator {

    real_t errorsum;
    int istep;


    __net_eval_decl Evaluator(){ }

    __net_eval_decl double FitnessFunction(CpuNetwork* net, int n_evals, int initial_seed){return -DBL_MAX;}

    __net_eval_decl void FitnessFunction_parallel(CpuNetwork* net, int n_reevals, double* res, int initial_seed){ }

    __net_eval_decl void execute(class Network **nets_, OrganismEvaluation *results, size_t nnets) { std::cout << "error, no execute defined" << endl; exit(1);}
    
    __net_eval_decl bool next_step() { }

    __net_eval_decl bool clear_noninput() { }

    __net_eval_decl real_t get_sensor(node_size_t sensor_index) { }

    __net_eval_decl void evaluate(real_t *actual) { }

    __net_eval_decl OrganismEvaluation result() { }


};


//---
//--- CLASS StaticNetworkEvaluator
//---
//--- Implementation of the NetworkEvaluator interface that serves to hide the
//--- Cuda code from the rest of the (C++11) system.
//---
class StaticNetworkEvaluator : public NetworkEvaluator {
    NetworkExecutor<Evaluator> *executor;
public:
    StaticNetworkEvaluator(const vector<Test> &tests) {
        executor = NetworkExecutor<Evaluator>::create();
    }

    ~StaticNetworkEvaluator() {
        delete executor;
    }

    virtual void execute(class Network **nets_,
                         class OrganismEvaluation *results,
                         size_t nnets) {
        executor->execute(nets_, results, nnets);
    }

    virtual void run_given_conf_file(std::string conf_file_path)
    {
       std::cout << "run_given_conf_file should not have been called at staticevaluator.cxx";
    }


};

namespace NEAT {
    NetworkEvaluator *create_static_evaluator(const vector<Test> &tests) {
        return new StaticNetworkEvaluator(tests);
    }
}
