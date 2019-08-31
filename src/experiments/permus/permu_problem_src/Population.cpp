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
    f_best = MIN_INTEGER;
    GenerateRandomPermutation(this->genome_best, n);


    m_individuals.resize(popsize);

    pop_info = new float *[popsize];
    permus = new int *[popsize]; // this contains the references to te permus in the individuals, so no initialization/destruction.

    //Initialize population with random solutions
    for (int i = 0; i < popsize; i++)
    {
        m_individuals[i] = new CIndividual(n);
    }

    for (int i = 0; i < popsize; i++)
    {
        pop_info[i] = new float[NEAT::__sensor_N];
    }
    pt = new PermuTools(n);
    timer = new stopwatch();
    terminated = false;
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
    if(timer->toc() > MAX_TIME)
    {
        terminated = true;
    }
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




  float* CPopulation::get_neat_input_individual_i(int i){
      return pop_info[i];
  }



void CPopulation::apply_neat_output_to_individual_i(float* output_neat, int i){
    if 
    (    
        (-CUTOFF_0 < output_neat[0] && output_neat[0] < CUTOFF_0) ||
        (sum_abs_val_slice_vec(output_neat, 1, 1+NEAT::N_OPERATORS) == 0) 
    )
    {
        return;
    }else if(output_neat[0] < -CUTOFF_0){ // Local-search iteration.
        //#TODO check if unconnected output is 0.
        int operator_id = argmax(output_neat + 1, NEAT::N_OPERATORS);
        this->problem->local_search_iteration(m_individuals[i], operator_id);
    }else if(output_neat[0] > CUTOFF_0){ // Move-with coeficients.
        int operator_id = argmax(output_neat + 1, NEAT::N_OPERATORS);
        float* coef = output_neat + (NEAT::__output_N - NEAT::N_COEF);
        this->move_individual_i_based_on_coefs(coef, i, operator_id);
    }
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
        float res =  (float)i / (float)POPSIZE;
        this->m_individuals[i]->relative_pos = res;
        pop_info[i][NEAT::RELATIVE_POSITION] = res;
    }
}

void CPopulation::comp_relative_time()
{
    for (int i = 0; i < POPSIZE; i++)
    {
        float res =  timer->toc() / MAX_TIME;
        this->m_individuals[i]->relative_time = res;
        pop_info[i][NEAT::RELATIVE_TIME] = res;
    }
    return ;
}

void CPopulation::comp_distance()
{

    // First, compute the distance of each permu with respect the  next permu
    pop_info[0][NEAT::DISTANCE] = (float)Hamming_distance(m_individuals[0]->genome, m_individuals[1]->genome, n);
    for (int i = 1; i < POPSIZE - 1; i++)
    {
        pop_info[i][NEAT::DISTANCE] = (float)Hamming_distance(m_individuals[i]->genome, m_individuals[i + 1]->genome, n);
    }
    pop_info[POPSIZE - 1][NEAT::DISTANCE] = pop_info[POPSIZE - 2][NEAT::DISTANCE];

    // Then, assign to result_vector[i], the minimun of the distances between the next an the prev permus.
    float distance_respect_to_previous = pop_info[0][NEAT::DISTANCE];
    float temp;
    for (int i = 1; i < POPSIZE - 1; i++)
    {
        temp = pop_info[i][NEAT::DISTANCE];
        pop_info[i][NEAT::DISTANCE] = MIN(pop_info[i][NEAT::DISTANCE], distance_respect_to_previous);
        distance_respect_to_previous = temp;
    }

    // Finally, normalize the values for them to be between 0 and 1.
    for (int i = 0; i < POPSIZE; i++)
    {
        pop_info[i][NEAT::DISTANCE] /= (float)n;
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
        m_individuals[i]->sparsity = 1.0 - (pt->get_distance_to_marginal(permus[i]) / n);
        pop_info[i][NEAT::SPARSITY] = m_individuals[i]->sparsity;
    }
}


void CPopulation::comp_r_number()
{
    for (int i = 0; i < POPSIZE; i++)
    {
        float res =  random_0_1_float();
        pop_info[i][NEAT::R_NUMBER] = res;
    }
}

void CPopulation::load_local_opt(){
for (int i = 0; i < POPSIZE; i++)
{
    pop_info[i][NEAT::OPT_SWAP] = (float) m_individuals[i]->is_local_optimum[NEAT::SWAP];
    pop_info[i][NEAT::OPT_EXCH] = (float) m_individuals[i]->is_local_optimum[NEAT::EXCH];
    pop_info[i][NEAT::OPT_INSERT] = (float) m_individuals[i]->is_local_optimum[NEAT::INSERT];
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
 

void CPopulation::move_individual_i_based_on_coefs(float* coef_list, int i, int operator_id){

    int idx = pt->choose_permu_index_to_move(coef_list);
    if (idx == -1){
        return;
    }

    bool towards = coef_list[idx] > 0;
    idx += (NEAT::__output_N - NEAT::N_COEF);

    int* ref_permu;
    
    switch (idx)
    {
    case NEAT::c_momentum:
        ref_permu = m_individuals[i]->momentum;
        break;
    case NEAT::c_pers_best:
        ref_permu = m_individuals[i]->genome_best;
        break;
    case NEAT::c_best_known:
        ref_permu = this->genome_best;
        break;
    case NEAT::c_above:
        if (i == 0)
        {
            ref_permu = this->m_individuals[1]->genome;
        }else
        {
            ref_permu = this->m_individuals[i-1]->genome;
        }
        break;
    
    default:
        cout << "error: a permutation must be chosen to move towards/away from.";
        exit(1);
        break;
    }

    if (idx != NEAT::c_momentum)
    {
        copy_vector(this->m_individuals[i]->momentum, ref_permu, n);
    }
    

    if(towards){
        problem->move_indiv_towards_reference(m_individuals[i], ref_permu, operator_id);
    }else
    {
        problem->move_indiv_away_reference(m_individuals[i], ref_permu, operator_id);
    }
}
