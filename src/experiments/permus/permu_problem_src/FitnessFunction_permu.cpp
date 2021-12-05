#include "std.hxx"
#include "networkexecutor.h"
#include "Parameters.h"
#include "PERMU_params.h"
#include "FitnessFunction_permu.h"
#include "map.h"
#include "network.h"
#include "resource.h"
#include <assert.h>
#include "../permus/permu_problem_src/PBP.h"
#include "../permus/permu_problem_src/Population.h"
#include "Tools.h"
#include <cfloat>
#include "../permus/permu_problem_src/QAP.h"
#include "../permus/permu_problem_src/LOP.h"
#include "../permus/permu_problem_src/PFSP.h"
#include "../permus/permu_problem_src/TSP.h"


// #define COUNTER 1
// #define PRINT 1

namespace PERMU{

static std::vector<PBP*> problem_list = {};
static std::vector<std::string> filenames = {};


static std::mutex mut_load_problem;
void _GetProblemInfo(std::string problemType, std::string filename, PBP** problem);
void GetProblemInfo(std::string problemType, std::string filename, PBP** problem)
{
    mut_load_problem.lock();
    bool loaded_cachedProblem = false;
    for (long i = 0; i < problem_list.size(); i++)
    {
        
        if (!problem_list[i]->is_being_used && filename == filenames[i])
        {
            (*problem) = problem_list[i];
            (*problem)->is_being_used = true;
            loaded_cachedProblem = true;
            break;
        }
    }
    if (!loaded_cachedProblem)
    {
        _GetProblemInfo(problemType, filename, problem);
        (*problem)->is_being_used = true;
        problem_list.push_back(*problem);
        filenames.push_back(filename);
        (*problem)->dbg_problem_index = problem_list.size()-1;
    }    
    mut_load_problem.unlock();
    (*problem)-> n_evals = 0;
	(*problem)-> n_evals_last = 0;
  	(*problem)-> n_iterations_with_same_n_evals = 0;
}




void _GetProblemInfo(std::string problemType, std::string filename, PBP** problem)
{
    if (problemType == "pfsp")
    {
        (*problem) = new PFSP();
    }
    else if (problemType == "tsp")
    {
        (*problem) = new TSP();
    }
    else if (problemType == "qap")
    {
        (*problem) = new QAP();
    }
    else if (problemType == "lop")
    {
        (*problem) = new LOP();
    }
    // else if (problemType == "api")
    //     problem = new API();
    else
    {
         cout << "Wrong problem type was specified. Problem type \"" << problemType << "\" not recognized."  << endl;
         exit(1);
     }

    //Read the instance.
    (*problem)->Read_with_mutex(filename);
    (*problem)-> n_evals = 0;
	(*problem)-> n_evals_last = 0;
  	(*problem)-> n_iterations_with_same_n_evals = 0;

}


//#define COUNTER
double FitnessFunction_permu(NEAT::CpuNetwork *net_original, uint32_t seed, PERMU::params* parameters)
{   
    using namespace PERMU;

    // cout << seed << ": ";

    PERMU::CPopulation *pop;
    PERMU::PBP *problem;

    NEAT::CpuNetwork tmp_net = *net_original;
    NEAT::CpuNetwork *net = &tmp_net;


    GetProblemInfo(parameters->PROBLEM_TYPE, parameters->INSTANCE_PATH, &problem);     //Read the problem instance to optimize.
    

    pop = new CPopulation(problem, parameters);
    problem->load_rng(pop->rng);
    pop->rng->seed(seed);


    #ifdef COUNTER
        int counter = 0;
    #endif


    double res = -DBL_MAX;
    {

        #ifdef COUNTER
                counter = 0;
        #endif


        pop->Reset();
        //std::cout << "|" << n_of_repetitions_completed << "|" << std::endl;

        net->clear_noninput();

        #ifdef RANDOM_SEARCH
        pop->timer->tic();
        double best_f = -DBL_MAX;
        GenerateRandomPermutation(pop->genome_best, pop->n, pop->rng);
        #endif


        while (!pop->terminated)
        {
            #ifdef COUNTER
            counter++;
            // if (counter < 3 || counter == 50)
            // {
            //     std::cout << "iteration number: " << counter << std::endl;
            #ifdef PRINT
            pop->Print();
            #endif
            // }
            #endif

            for (int i = 0; i < pop->popsize; i++)
            {

                for (int sns_idx = 0; sns_idx < PERMU::__sensor_N; sns_idx++)
                {
                    net->load_sensor(sns_idx, pop->get_neat_input_individual_i(i)[sns_idx]);
                }
                net->activate();
                pop->apply_neat_output_to_individual_i(net->get_outputs(), i);
                net->clear_noninput();
            }
            pop->end_iteration();
        }
        if (!isPermutation(pop->genome_best, pop->n))
        {
            cout << "final result is not permutation" << endl;
            cout << "final permu: ";
            PrintArray(pop->genome_best, pop->n);
            exit(1);
        }
        res = problem->Evaluate(pop->genome_best);
        net->clear_noninput();
        #ifdef COUNTER
        cout << counter << endl;
        #endif
    }



    delete pop;
    pop = NULL;
    problem->is_being_used = false;
    net = NULL;
    // cout << res << "," << std::endl;
    return res;
}

}