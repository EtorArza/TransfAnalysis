#pragma once

namespace NEAT {

// a even number is expected here. The maximum layer depth of the network
// is limited by this number
#define NACTIVATES_PER_INPUT 6

    //---
    //--- CLASS NetworkExecutor<>
    //---
    template<typename Evaluator>
    class NetworkExecutor {
    public:
        static NetworkExecutor *create();
        
        virtual ~NetworkExecutor() {}


        virtual void execute(class Network **nets_,
                             class OrganismEvaluation *results,
                             size_t nnets) = 0;
    };

}

#ifdef ENABLE_CUDA
#include "cudanetworkexecutor.h"
#else
#include "cpunetworkexecutor.h"
#endif
