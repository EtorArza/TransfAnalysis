//
//  Population.cc
//  RankingEDAsCEC
//
//  Created by Josu Ceberio Uribe on 11/19/13.
//  Copyright (c) 2013 Josu Ceberio Uribe. All rights reserved.
//

#include "Population.h"
#include "PBP.h"
#include "Parameters.h"
#include "Tools.h"

using std::cerr;
using std::cout;
using std::endl;
using std::istream;
using std::ostream;

#define RELATIVE_POSITION 0
#define RELATIVE_TIME 1
#define DISTANCE 2
#define SPARSITY 3

/*
 * Constructor function.
 */
CPopulation::CPopulation(int popsize, int n)
{
    this->popsize = popsize;
    this->n = n;
    m_individuals.resize(popsize);

    pop_info = new double *[popsize];
    permus = new int *[popsize]; // this contains the references to te permus in the individuals, so no initialization/destruction.

    //Initialize population with random solutions
    for (int i = 0; i < popsize; i++)
    {
        m_individuals[i] = new CIndividual(n);
    }

    for (int i = 0; i < popsize; i++)
    {
        pop_info[i] = new double[LOGIC_INPUT_LEN];
    }
    pt = new PermuTools(n);
    timer = new stopwatch();
}

/*
 * Destructor function.
 */
CPopulation::~CPopulation()
{
    for (int i = 0; i < POPSIZE; i++)
    {
        delete m_individuals[i];
        delete[] pop_info[i];
    }
    m_individuals.clear();
    delete timer;
    delete pt;
    delete[] pop_info;
    delete[] permus;
}



/*
 * Prints the current population.
 */
void CPopulation::Print()
{   cout << "---" << endl;
    for (int i = 0; i < popsize; i++)
        cout << m_individuals[i] << endl;
    cout << "---" << endl;
}

bool CPopulation::Exists(int *permutation, int problem_size, int sel_size)
{
    for (int i = 0; i < sel_size; i++)
    {
        if (memcmp(permutation, m_individuals[i]->genome, sizeof(int) * problem_size) == 0)
        {
            return true;
        }
    }
    return false;
}

/*
 * Calculates the average fitness of the first 'len' solutions in the population
 */
double CPopulation::AverageFitnessPopulation(int len)
{
    double result = 0;
    for (int i = 0; i < len; i++)
    {
        result += m_individuals[i]->f_value;
    }
    return result / len;
}

/*
 * Sorts the individuals in the population in decreasing order of the fitness value.
 */
void CPopulation::SortPopulation()
{
    sort(m_individuals.begin(), m_individuals.end(), Better);
}

// Evaluate the whole population
void CPopulation::evaluate_population(PBP *problem)
{
    for (int i = 0; i < popsize; i++)
    {
        problem->Evaluate(m_individuals[i]);
    }
}


void CPopulation::get_population_info(){
    comp_relative_position();
    comp_relative_time();
    comp_distance();
    comp_sparsity();
}


void CPopulation::comp_relative_position()
{
    for (int i = 0; i < POPSIZE; i++)
    {
        double res =  (double)i / (double)POPSIZE;
        this->m_individuals[i]->relative_pos = res;
        pop_info[i][RELATIVE_POSITION] = res;
    }
}

void CPopulation::comp_relative_time()
{
    for (int i = 0; i < POPSIZE; i++)
    {
        double res =  timer->toc();
        this->m_individuals[i]->relative_pos = res;
        pop_info[i][RELATIVE_TIME] = res;
    }
    return ;
}

void CPopulation::comp_distance()
{

    // First, compute the distance of each permu with respect the  next permu
    pop_info[0][DISTANCE] = (double)Hamming_distance(m_individuals[0]->genome, m_individuals[1]->genome, n);
    for (int i = 1; i < POPSIZE - 1; i++)
    {
        pop_info[i][DISTANCE] = (double)Hamming_distance(m_individuals[i]->genome, m_individuals[i + 1]->genome, n);
    }
    pop_info[POPSIZE - 1][DISTANCE] = pop_info[POPSIZE - 2][DISTANCE];

    // Then, assign to result_vector[i], the minimun of the distances between the next an the prev permus.
    double distance_respect_to_previous = pop_info[0][DISTANCE];
    double temp;
    for (int i = 1; i < POPSIZE - 1; i++)
    {
        temp = pop_info[i][DISTANCE];
        pop_info[i][DISTANCE] = MIN(pop_info[i][DISTANCE], distance_respect_to_previous);
        distance_respect_to_previous = temp;
    }

    // Finally, normalize the values for them to be between 0 and 1.
    for (int i = 0; i < POPSIZE; i++)
    {
        pop_info[i][DISTANCE] /= (double)n;
    }

    // copy values into individuals
    for (int i = 0; i < POPSIZE; i++)
    {
        m_individuals[i]->distance = pop_info[i][DISTANCE];
    }
}


void CPopulation::comp_sparsity(){
    copy_references_of_genomes_from_individuals_to_permus();
    pt->compute_first_marginal(permus, POPSIZE);
    for (int i = 0; i < n; i++)
    {
        m_individuals[i]->sparsity = pt->get_distance_to_marginal(permus[i]);
        pop_info[i][SPARSITY] = m_individuals[i]->sparsity;
    }
}


// void CPopulation::take_action_with_action_id(int permutation_index, int action_id, PBP *problem)
// {
//     switch (action_id)
//     {
//     case ACTION_1_LOCAL_SEARCH_ITERATION:
//         problem->local_search_iteration_insertion(m_individuals[permutation_index]);
//         break;
//     case ACTION_2_MOVE_AWAY_FROM_WORSE:
//         if (permutation_index != POPSIZE - 1)
//         {
//             problem->move_permutation_away_reference_with_insertion(m_individuals[permutation_index], m_individuals[permutation_index + 1]);
//         }
//         break;
//     case ACTION_3_MOVE_TOWARDS_BEST:
//         if (permutation_index != 0)
//         {
//             problem->move_permutation_towards_reference_with_insertion(m_individuals[permutation_index], m_individuals[0]);
//         }
//         break;
//     case ACTION_4_MOVE_TOWARDS_BETTER:
//         if (permutation_index != 0)
//         {
//             problem->move_permutation_towards_reference_with_insertion(m_individuals[permutation_index], m_individuals[permutation_index - 1]);
//         }
//         break;
//     default:
//         cout << "ERROR, action_id not recognized" << endl;
//         exit(1);
//     }
// }



void CPopulation::copy_references_of_genomes_from_individuals_to_permus(){
    for (int i = 0; i < POPSIZE; i++)
    {
        permus[i] = m_individuals[i]->genome;
    }
}
