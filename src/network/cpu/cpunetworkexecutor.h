#pragma once

#include "cpunetwork.h"
#include "Parameters.h"
#include <mutex>


namespace NEAT {

    //Don't need any special qualifiers for CPU
    #define __net_eval_decl

    //---
    //--- CLASS CpuNetworkExecutor
    //---
    template<typename Evaluator>
    class CpuNetworkExecutor : public NetworkExecutor<Evaluator> {
    public:
        const typename Evaluator::Config *config;

        CpuNetworkExecutor() {
            config = NULL;
        }

        virtual ~CpuNetworkExecutor() {
            delete config;
        }

        virtual void configure(const typename Evaluator::Config *config_,
                               size_t len) {
            void *buf = malloc(len);
            memcpy(buf, config_, len);
            config = (const typename Evaluator::Config *)buf;
        }

        virtual void execute(class Network **nets_,
                             OrganismEvaluation *results,
                             size_t nnets) {
            
            CpuNetwork **nets = (CpuNetwork **)nets_;
            double progress_print_decider = 0.0;
            bool printed_bracket = false;
            #pragma omp parallel for
            for(size_t inet = 0; inet < nnets; inet++) {
                CpuNetwork *net = nets[inet];
                Evaluator *ev = new Evaluator(config);
                OrganismEvaluation eval;
                eval.fitness = ev->FitnessFunction(net);
                eval.error   = -100000000 + eval.fitness;
                results[inet] = eval;
                delete ev;

                // print progress.
                std::mutex mut;
                mut.lock();
                if (!printed_bracket){std::cout << "[" << std::flush; printed_bracket = true;}
                progress_print_decider += 15.0 / (double) nnets;
                if (inet == 0){}
                while (progress_print_decider >= 1.0)
                {
                    std::cout << "." << std::flush;
                    progress_print_decider--;
                }
                mut.unlock();
            }
            std::cout << "]" << std::endl;
        }


    };

    //---
    //--- FUNC NetworkExecutor<Evaluator>::create()
    //---
    template<typename Evaluator>
    inline NetworkExecutor<Evaluator> *NetworkExecutor<Evaluator>::create() {
        return new CpuNetworkExecutor<Evaluator>();
    }
}
