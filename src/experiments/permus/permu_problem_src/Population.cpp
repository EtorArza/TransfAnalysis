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
#include "../permuevaluator.h"

using std::cerr;
using std::cout;
using std::endl;
using std::istream;
using std::ostream;


/*
 * Constructor function.
 */
CPopulation::CPopulation(PBP *problem)
{   this->problem = problem;
    this->popsize = POPSIZE;
    this->n = problem->GetProblemSize();

    genome_best = new int[n];
    GenerateRandomPermutation(this->genome_best, n);


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
        pop_info[i] = new double[NEAT::__sensor_N];
    }
    pt = new PermuTools(n);
    timer = new stopwatch();

    evaluate_population();
    end_iteration();
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
    delete[] genome_best;
}



void CPopulation::end_iteration(){
    SortPopulation();
    get_population_info();
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




  double* CPopulation::get_neat_input_individual_i(int i){
      return pop_info[i];
  }



  void CPopulation::apply_neat_output_to_individual_i(double* output_neat, int i){
      
      if (-CUTOFF_0 < output_neat[0] < CUTOFF_0)
      {
          return;
      }else if(output_neat[0] < -CUTOFF_0){ // Local-search iteration.
          //#TODO check if unconnected output is 0.
          if(sum_abs_val_slice_vec(output_neat, 1, 1+NEAT::N_OPERATORS) == 0){
              return;
          }
          int operator_id = argmax(output_neat + 1, NEAT::N_OPERATORS);
          this->problem->local_search_iteration(m_individuals[i], operator_id);
      }else if(output_neat[0] > CUTOFF_0){ // Move-towards

      }
      
      double* coef = output_neat + (NEAT::__output_N - NEAT::N_COEF);
  }




/*
 * Sorts the individuals in the population in decreasing order of the fitness value.
 */
void CPopulation::SortPopulation()
{
    sort(m_individuals.begin(), m_individuals.end(), Better);
    if (m_individuals[0]->f_value > this->f_best)
    {
        this->f_best = m_individuals[0]->f_value;
        copy_vector(this->genome_best, m_individuals[0]->genome, n);
    }
    
}

// Evaluate the whole population
void CPopulation::evaluate_population()
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
    comp_r_number();
    load_local_opt();
}


void CPopulation::comp_relative_position()
{
    for (int i = 0; i < POPSIZE; i++)
    {
        double res =  (double)i / (double)POPSIZE;
        this->m_individuals[i]->relative_pos = res;
        pop_info[i][NEAT::RELATIVE_POSITION] = res;
    }
}

void CPopulation::comp_relative_time()
{
    for (int i = 0; i < POPSIZE; i++)
    {
        double res =  timer->toc() / MAX_TIME;
        this->m_individuals[i]->relative_time = res;
        pop_info[i][NEAT::RELATIVE_TIME] = res;
    }
    return ;
}

void CPopulation::comp_distance()
{

    // First, compute the distance of each permu with respect the  next permu
    pop_info[0][NEAT::DISTANCE] = (double)Hamming_distance(m_individuals[0]->genome, m_individuals[1]->genome, n);
    for (int i = 1; i < POPSIZE - 1; i++)
    {
        pop_info[i][NEAT::DISTANCE] = (double)Hamming_distance(m_individuals[i]->genome, m_individuals[i + 1]->genome, n);
    }
    pop_info[POPSIZE - 1][NEAT::DISTANCE] = pop_info[POPSIZE - 2][NEAT::DISTANCE];

    // Then, assign to result_vector[i], the minimun of the distances between the next an the prev permus.
    double distance_respect_to_previous = pop_info[0][NEAT::DISTANCE];
    double temp;
    for (int i = 1; i < POPSIZE - 1; i++)
    {
        temp = pop_info[i][NEAT::DISTANCE];
        pop_info[i][NEAT::DISTANCE] = MIN(pop_info[i][NEAT::DISTANCE], distance_respect_to_previous);
        distance_respect_to_previous = temp;
    }

    // Finally, normalize the values for them to be between 0 and 1.
    for (int i = 0; i < POPSIZE; i++)
    {
        pop_info[i][NEAT::DISTANCE] /= (double)n;
    }

    // copy values into individuals
    for (int i = 0; i < POPSIZE; i++)
    {
        m_individuals[i]->distance = pop_info[i][NEAT::DISTANCE];
    }
}


void CPopulation::comp_sparsity(){
    copy_references_of_genomes_from_individuals_to_permus();
    pt->compute_first_marginal(permus, POPSIZE);
    for (int i = 0; i < POPSIZE; i++)
    {
        m_individuals[i]->sparsity = 1.0 - (pt->get_distance_to_marginal(permus[i]) / POPSIZE);
        pop_info[i][NEAT::SPARSITY] = m_individuals[i]->sparsity;
    }
}


void CPopulation::comp_r_number()
{
    for (int i = 0; i < POPSIZE; i++)
    {
        double res =  random_0_1_float();
        this->m_individuals[i]->relative_pos = res;
        pop_info[i][NEAT::R_NUMBER] = res;
    }
}

void CPopulation::load_local_opt(){
for (int i = 0; i < POPSIZE; i++)
{
    pop_info[i][NEAT::OPT_SWAP] = (double) m_individuals[i]->is_local_optimum[NEAT::SWAP];
    pop_info[i][NEAT::OPT_EXCH] = (double) m_individuals[i]->is_local_optimum[NEAT::EXCH];
    pop_info[i][NEAT::OPT_INSERT] = (double) m_individuals[i]->is_local_optimum[NEAT::INSERT];
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
