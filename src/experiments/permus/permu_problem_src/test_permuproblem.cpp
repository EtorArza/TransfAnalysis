//
//  main.cpp
//  RankingEDAsCEC
//
//  Created by Josu Ceberio Uribe on 11/19/13.
//  Copyright (c) 2013 Josu Ceberio Uribe. All rights reserved.
//
#define DEFINE_GLOBALS
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

// It is the instance of the problem to optimize.
PBP *problem;
CPopulation *pop;

// Reads the problem info of the instance set.

PBP *GetProblemInfo(string problemType, string filename)
{
    PBP *problem;
    // if (problemType == "PFSP")
    //     problem = new PFSP();
    // else if (problemType == "TSP")
    //     problem = new TSP();
    // else if (problemType == "QAP")
    //     problem = new QAP();
    // else if (problemType == "LOP")
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

    //Read the problem instance to optimize.
    problem = GetProblemInfo(PROBLEM_TYPE, INSTANCE_PATH);
    pop = new CPopulation(POPSIZE, problem->GetProblemSize());

    pop->evaluate_population(problem);
    pop->SortPopulation();

    std::cout << "testing move_towards";
    for (int k = 0; k < 100; k++)
    {
        std::cout << ".";
        for (int operator_id = 1; operator_id < 4; operator_id++)
        {
            for (int i = 1; i < POPSIZE; i++)
            {
                problem->move_indiv_towards_reference(pop->m_individuals[i], pop->m_individuals[0]->genome, operator_id);
                int f_moved = pop->m_individuals[i]->f_value;
                problem->Evaluate(pop->m_individuals[i]);
                if (f_moved != pop->m_individuals[i]->f_value)
                {
                    std::cout << " failed." << endl;
                    exit(1);
                }
            }
        }
    }
    std::cout << "-> passed" << endl;
    delete pop;
    pop = new CPopulation(POPSIZE, problem->GetProblemSize());
    pop->evaluate_population(problem);
    pop->SortPopulation();



    std::cout << "testing local search";
    for (int k = 0; k < 100; k++)
    {
        std::cout << ".";
        for (int operator_id = 1; operator_id < 4; operator_id++)
        {
            for (int i = 0; i < POPSIZE; i++)
            {
                int orig = pop->m_individuals[i]->f_value;
                problem->local_search_iteration(pop->m_individuals[i], operator_id);              
                int f_moved = pop->m_individuals[i]->f_value;
                if (f_moved < orig)
                {
                    std::cout << " failed, sol not improved with ls." << endl;
                    exit(1);
                }
                problem->Evaluate(pop->m_individuals[i]);
                if (f_moved != pop->m_individuals[i]->f_value)
                {
                    std::cout << " failed, sol update not equal to directly evaluating sol." << endl;
                    exit(1);
                }
            }
        }
    }
    std::cout << "-> passed" << endl;
    delete pop;
    pop = new CPopulation(POPSIZE, problem->GetProblemSize());
    pop->evaluate_population(problem);
    pop->SortPopulation();


    std::cout << "testing permu combination";
    for (int k = 0; k < 100; k++)
    {
        std::cout << ".";
        int n = 1000;
        PermuTools* pt = new PermuTools(n);
        int *permu1 = new int[n];
        int *permu2 = new int[n];
        int *permu3 = new int[n];
        int *permu0 = new int[n];
        int *res = new int[n];

        GenerateRandomPermutation(permu0, n);
        for (int i = 0; i < n; i++)
        {
            permu1[i] = permu0[i];
            permu2[i] = permu0[i];
            permu3[i] = permu0[i];
        }

        shuffle_vector(permu1, n);
        shuffle_vector(permu2, n);


        double coef[4] = {0.9, 0.1, -0.01, -0.01};
        int *permus[4] = {permu0, permu1, permu2, permu3};
        
        pt->combine_permus(permus, coef, 4, res);

        if (Hamming_distance(permu0, res, n) > 0.5*(double)n)
        {
            std::cout << " failed, sol too far away from reference(1)." << endl;
            exit(1);
        }

        double coef2[4] = {0.2, 0.2, -0.3, -0.3};
        pt->combine_permus(permus, coef2, 4, res);

        if (Hamming_distance(permu1, res, n) > 0.7*(double)n)
        {
            std::cout << " failed, sol too far away from reference(2)." << endl;
            exit(1);
        }

        if (Hamming_distance(permu0, res, n) < 0.5*(double)n)
        {
            std::cout << " failed, sol close away from reference." << endl;
            exit(1);
        }


    }
    std::cout << "-> passed" << endl;


    pop->evaluate_population(problem);
    pop->SortPopulation();
    pop->get_population_info();

    pop->Print();

    std::cout << "--- " << timer.toc() << " ---" << endl;

    return 0;
}
