
#include "std.hxx"
#include "networkexecutor.h"
#include "Parameters.h"
#include "FitnessFunction_permu.h"
#include "map.h"
#include "network.h"
#include "resource.h"
#include <assert.h>
#include "PBP.h"
#include "Population.h"
#include "Tools.h"
#include <cfloat>
#include "QAP.h"
#include "LOP.h"
#include "PFSP.h"
#include "TSP.h"


PBP *GetProblemInfo(std::string problemType, std::string filename)
{
    PBP *problem;
    if (problemType == "pfsp")
    {
        problem = new PFSP();
    }
    else if (problemType == "tsp")
    {
        problem = new TSP();
    }
    else if (problemType == "qap")
    {
        problem = new QAP();
    }
    else if (problemType == "lop")
    {
        problem = new LOP();
    }
    // else if (problemType == "api")
    //     problem = new API();
    else
    {
         cout << "Wrong problem type was specified." << endl;
         exit(1);
     }

    //Read the instance.
    problem->Read_with_mutex(filename);
    #ifdef SAME_SIZE_EXPERIMENT
        if (problem->GetProblemSize() == 30)
        {
            MAX_TIME_PSO = 0.10;
        }
        else if(problem->GetProblemSize() == 60)
        {
            MAX_TIME_PSO = 0.3;
        }
        else
        {
            cout << "ERROR, this experiment expects instances of size 60 and 30.";
            exit(1);
        }

    #undef SAME_SIZE_EXPERIMENT
    #endif
    return problem;
}



double FitnessFunction_permu(NEAT::CpuNetwork *net_original, int n_evals, int seed)
{

    double *v_of_fitness;
    PBP *problem;
    CPopulation *pop;

    NEAT::CpuNetwork tmp_net = *net_original;
    NEAT::CpuNetwork *net = &tmp_net;

 
    problem = GetProblemInfo(PROBLEM_TYPE, INSTANCE_PATH);     //Read the problem instance to optimize.
    pop = new CPopulation(problem);
    problem->load_rng(pop->rng);
    pop->rng->seed(seed);


    v_of_fitness = new double[n_evals];

    for (int i = 0; i < POPSIZE; i++)
    {
        pop->m_individuals[i]->activation = std::vector<double>(net->activations);
    }

    #ifdef COUNTER
        int counter = 0;
    #endif

    for (int n_of_repetitions_completed = 0; n_of_repetitions_completed < n_evals; n_of_repetitions_completed++)
    {

        #ifdef COUNTER
                counter = 0;
        #endif


        pop->rng->seed(seed + n_of_repetitions_completed);
        pop->Reset();
        //std::cout << "|" << n_of_repetitions_completed << "|" << std::endl;
        for (int i = 0; i < POPSIZE; i++)
        {
            std::swap(net->activations, pop->m_individuals[i]->activation);
            net->clear_noninput();
            std::swap(net->activations, pop->m_individuals[i]->activation);
        }


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


            #ifdef RANDOM_SEARCH
            double new_f = problem->Evaluate(pop->genome_best);
            GenerateRandomPermutation(pop->genome_best, pop->n, pop->rng);
            if (new_f > best_f)
            {
                best_f = new_f;
            }
            if(pop->timer->toc() > MAX_TIME_PSO)
            {
                pop->terminated = true;
            }
            continue;
            #endif


            for (int i = 0; i < POPSIZE; i++)
            {

                std::swap(net->activations, pop->m_individuals[i]->activation);
                for (int sns_idx = 0; sns_idx < NEAT::__sensor_N; sns_idx++)
                {
                    net->load_sensor(sns_idx, pop->get_neat_input_individual_i(i)[sns_idx]);
                }
                net->activate();
                pop->apply_neat_output_to_individual_i(net->get_outputs(), i);
                std::swap(net->activations, pop->m_individuals[i]->activation);
            }
            pop->end_iteration();
            //pop->Print();
        }
        if (!isPermutation(pop->genome_best, pop->n))
        {
            cout << "final result is not permutation" << endl;
            cout << "final permu: ";
            PrintArray(pop->genome_best, pop->n);
            exit(1);
        }
        v_of_fitness[n_of_repetitions_completed] = problem->Evaluate(pop->genome_best);
        net->clear_noninput();
        #ifdef COUNTER
        cout << counter << endl;
        #endif
    }

    double res = Average(v_of_fitness, n_evals);


    delete[] v_of_fitness;
    delete pop;
    delete problem;
    pop = NULL;
    v_of_fitness = NULL;
    problem = NULL;
    net = NULL;
    return res;
}