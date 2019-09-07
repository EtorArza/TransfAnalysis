//
//  main.cpp
//  RankingEDAsCEC
//
//  Created by Josu Ceberio Uribe on 11/19/13.
//  Copyright (c) 2013 Josu Ceberio Uribe. All rights reserved.
//
#define EXTERN
#include "Parameters.h"
#include <sys/time.h>
#include <iostream>
#include <iomanip>
#include "PBP.h"
//#include "PFSP.h"
#include "LOP.h"
//#include "QAP.h"
//#include "TSP.h"
//#include "API.h"
#include "Population.h"
#include "Tools.h"
#include "loadnetworkfromfile.h"
#include "organism.h"

// It is the instance of the problem to optimize.
PBP *problem;
CPopulation *pop;

// Reads the problem info of the instance set.

PBP *GetProblemInfo(std::string problemType, std::string filename)
{
    PBP *problem;
    // if (problemType == "PFSP")
    //     problem = new PFSP();
    // else if (problemType == "TSP")
    //     problem = new TSP();
    // else if (problemType == "QAP")
    //     problem = new QAP();
    // else 
    if (problemType == "lop")
    problem = new LOP();
    // else if (problemType == "API")
    //     problem = new API();
    // else
    // {
    //     cout << "Wrong problem type was specified." << endl;
    //     exit(1);
    // }

    //Read the instance.
    problem->Read(filename);
    return problem;
}

int main(int argc, char *argv[])
{
    cout << "begin" << endl;
    auto timer = stopwatch();
    timer.tic();                // Initialize time variables.
    set_parameters(argc, argv); // Read parameters from bash.

    NEAT::Organism* org;

    org = NEAT::load_organism(CONTROLLER_PATH);

    cout << "loaded";
    exit(0);





    // std::cout << "testing move_towards";
    // for (int k = 0; k < 100; k++)
    // {
    //     std::cout << ".";
    //     for (int operator_id = 1; operator_id < 3; operator_id++)
    //     {
    //         for (int i = 1; i < POPSIZE; i++)
    //         {
    //             problem->move_indiv_towards_reference(pop->m_individuals[i], pop->m_individuals[0]->genome, operator_id);
    //             int f_moved = pop->m_individuals[i]->f_value;
    //             problem->Evaluate(pop->m_individuals[i]);
    //             if (f_moved != pop->m_individuals[i]->f_value)
    //             {
    //                 std::cout << " failed." << endl;
    //                 exit(1);
    //             }
    //         }
    //     }
    // }
    // std::cout << "-> passed" << endl;
    // delete pop;
    // pop = new CPopulation(problem);




    // std::cout << "testing local search";
    // for (int k = 0; k < 100; k++)
    // {
    //     std::cout << ".";
    //     for (int operator_id = 0; operator_id < 3; operator_id++)
    //     {
    //         for (int i = 0; i < POPSIZE; i++)
    //         {
    //             int orig = pop->m_individuals[i]->f_value;
    //             problem->local_search_iteration(pop->m_individuals[i], operator_id);              
    //             int f_moved = pop->m_individuals[i]->f_value;
    //             if (f_moved < orig)
    //             {
    //                 std::cout << " failed, sol not improved with ls." << endl;
    //                 exit(1);
    //             }
    //             problem->Evaluate(pop->m_individuals[i]);
    //             if (f_moved != pop->m_individuals[i]->f_value)
    //             {
    //                 std::cout << " failed, sol update not equal to directly evaluating sol." << endl;
    //                 exit(1);
    //             }
    //         }
    //     }
    // }
    // std::cout << "-> passed" << endl;
    // delete pop;
    // pop = new CPopulation(problem);



    // std::cout << "testing permu combination";
    // for (int k = 0; k < 100; k++)
    // {
    //     std::cout << ".";
    //     int n = 1000;
    //     PermuTools* pt = new PermuTools(n);
    //     int *permu1 = new int[n];
    //     int *permu2 = new int[n];
    //     int *permu3 = new int[n];
    //     int *permu0 = new int[n];
    //     int *res = new int[n];

    //     GenerateRandomPermutation(permu0, n);
    //     for (int i = 0; i < n; i++)
    //     {
    //         permu1[i] = permu0[i];
    //         permu2[i] = permu0[i];
    //         permu3[i] = permu0[i];
    //     }

    //     shuffle_vector(permu1, n);
    //     shuffle_vector(permu2, n);


    //     float coef[4] = {0.9, 0.1, -0.01, -0.01};
    //     int *permus[4] = {permu0, permu1, permu2, permu3};
        
    //     pt->combine_permus(permus, coef, res);

    //     if (Hamming_distance(permu0, res, n) > 0.5*(float)n)
    //     {
    //         std::cout << " failed, sol too far away from reference(1)." << endl;
    //         exit(1);
    //     }

    //     float coef2[4] = {0.2, 0.2, -0.3, -0.3};
    //     pt->combine_permus(permus, coef2,  res);

    //     if (Hamming_distance(permu1, res, n) > 0.7*(float)n)
    //     {
    //         std::cout << " failed, sol too far away from reference(2)." << endl;
    //         exit(1);
    //     }

    //     if (Hamming_distance(permu0, res, n) < 0.5*(float)n)
    //     {
    //         std::cout << " failed, sol close away from reference." << endl;
    //         exit(1);
    //     }
    //     delete pt;

    // }
    // std::cout << "-> passed" << endl;


    float neat_output[8] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
    timer.tic();
    pop->f_best = 1000000000;
    for (int k = 0; k < 1000; k++)
    {
        pop->end_iteration();
        if (timer.toc() > 1.0)
        {
            pop->Print();
            timer.tic();
        }

        for (int i = 0; i < POPSIZE; i++)
        {   
            for (int z = 0; z < 8; z++)
            {
                neat_output[z] = (random_0_1_float()-0.5)*2;
            }
            neat_output[6] += 2;
            neat_output[0] -= 0.5;
            pop->apply_neat_output_to_individual_i(neat_output, i);
        
        }
    }


    pop->Print();
    delete pop;
    std::cout << "--- " << timer.toc() << " ---" << endl;

    return 0;
}
