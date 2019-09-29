#pragma once

#include "cpunetwork.h"
#include "Parameters.h"
#include "Tools.h"
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
            double* f_values = new double[nnets];
            bool printed_bracket = false;



            // evaluate the individuals 
            #pragma omp parallel for num_threads(N_OF_THREADS)
            for(size_t inet = 0; inet < nnets; inet++) {
                CpuNetwork *net = nets[inet];
                Evaluator *ev = new Evaluator(config);
                OrganismEvaluation eval;
                eval.fitness = ev->FitnessFunction(net);
                f_values[inet] = eval.fitness;
                eval.error   = -100000000 + eval.fitness;
                results[inet] = eval;
                delete ev;

                // print progress.
                std::mutex mutx;
                mutx.lock();
                if (!printed_bracket){std::cout << "[" << std::flush; printed_bracket = true;}
                progress_print_decider += 15.0 / (double) nnets;
                if (inet == 0){}
                while (progress_print_decider >= 1.0)
                {
                    std::cout << "." << std::flush;
                    progress_print_decider--;
                }
                mutx.unlock();
            }
            std::cout << "]" << std::endl;

            // // reevaluate top n_of_threads_omp, with a minimum of 5 and a maximum of nnets.
            // double cut_value = obtain_kth_largest_value(f_values, min(max(n_of_threads_omp, 5), static_cast<int>(nnets)), nnets);
         
            // reevaluate top 1% at least N_REEVAL times
            int actual_n_reevals = (( (N_REEVALS-1) / N_OF_THREADS) + 1) * N_OF_THREADS;
            int n_of_networks_to_reevaluate = max(1, static_cast<int>(nnets) / 100);
            cout << "reevaluating top 1% (" << n_of_networks_to_reevaluate << " nets out of " << static_cast<int>(nnets) << ") each "  << actual_n_reevals << " times." << endl;

            double cut_value = obtain_kth_largest_value(f_values, n_of_networks_to_reevaluate, static_cast<int>(nnets));

            for(size_t inet = 0; inet < nnets; inet++) {
                if (f_values[inet] < cut_value)
                {   
                    results[inet].fitness -= 100000000.0;
                    results[inet].error += 100000000.0;
                    f_values[inet] -= 100000000.0; // apply a discount to the individuals that are not reevaluated
                    continue;
                }
                else
                {
                    CpuNetwork *net = nets[inet];
                    Evaluator *ev = new Evaluator(config);
                    OrganismEvaluation eval;
                    eval.fitness = ev->FitnessFunction_reevaluation(net, actual_n_reevals);
                    f_values[inet] = eval.fitness;
                    eval.error = -100000000 + eval.fitness;
                    results[inet] = eval;
                    delete ev;
                }
            }
            delete[] f_values;
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
