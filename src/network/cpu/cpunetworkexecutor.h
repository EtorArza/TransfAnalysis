#pragma once

#include "cpunetwork.h"
#include "Parameters.h"
#include "Tools.h"
#include <mutex>

namespace NEAT
{

//Don't need any special qualifiers for CPU
#define __net_eval_decl

//---
//--- CLASS CpuNetworkExecutor
//---
template <typename Evaluator>
class CpuNetworkExecutor : public NetworkExecutor<Evaluator>
{
public:

    CpuNetworkExecutor()
    {
    }

    virtual ~CpuNetworkExecutor()
    {
    }


    void execute(class Network **nets_, OrganismEvaluation *results, size_t nnets)
    {
        std::cerr << "error, the execute function inside the";
        std::cerr << "custom evaluator class needs to be used instead of thisone. exiting..." << endl;
        exit(1);
    }
};

//---
//--- FUNC NetworkExecutor<Evaluator>::create()
//---
template <typename Evaluator>
inline NetworkExecutor<Evaluator> *NetworkExecutor<Evaluator>::create()
{
    return new CpuNetworkExecutor<Evaluator>();
    }
}
