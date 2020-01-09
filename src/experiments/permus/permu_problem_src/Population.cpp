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
#include "Tabu.h"
#include "../permuevaluator.h"
#include <assert.h>
#include <float.h>
#include "PERMU_params.h"

using std::cerr;
using std::cout;
using std::endl;
using std::istream;
using std::ostream;

#define TARGET_N_ITERATIONS_iteration_geom 10000
#define TARGET_VALUE_iteration_geom 0.5


namespace PERMU{

void CPopulation::init_class(PBP *problem, RandomNumberGenerator* rng, PERMU::params* parameters){
    this->rng = rng;
    this->problem = problem;
    this->popsize = parameters-> POPSIZE;
    this->max_time_pso = parameters-> MAX_TIME_PSO;
    this->iteration_geom = 1.0;
    this->iteration_geom_coef = pow(TARGET_VALUE_iteration_geom, 1.0/ ((double) TARGET_N_ITERATIONS_iteration_geom * parameters-> MAX_TIME_PSO));
    this->n = problem->GetProblemSize();
    tab = new Tabu(rng, n, parameters-> TABU_LENGTH);
    problem->tab = this->tab;
    genome_best = new int[n];
    f_best = -DBL_MAX;
    GenerateRandomPermutation(this->genome_best, n, this->rng);
    templ_double_array = new double[this->popsize];
    templ_double_array2 = new double[this->popsize];

    m_individuals.resize(popsize);

    pop_info = new double *[popsize];
    permus = new int *[popsize]; // this contains the references to te permus in the individuals, so no initialization/destruction.

    //Initialize population with random solutions
    for (int i = 0; i < popsize; i++)
    {
        m_individuals[i] = new CIndividual(n, this->rng);
    }

    for (int i = 0; i < popsize; i++)
    {
        pop_info[i] = new double[NEAT::__sensor_N];
    }
    pt = new PermuTools(n, rng);
    timer = new stopwatch();
    terminated = false;
    evaluate_population();
    end_iteration();
}

CPopulation::CPopulation(PBP *problem, PERMU::params* parameters)
{
    RandomNumberGenerator* tmp_rng = new RandomNumberGenerator();
    init_class(problem, tmp_rng, parameters);
}


CPopulation::CPopulation(PBP *problem, RandomNumberGenerator* rng, PERMU::params* parameters)
{
    init_class(problem, rng, parameters);
}

void CPopulation::Reset(){
    f_best = -DBL_MAX;
    GenerateRandomPermutation(this->genome_best, n, rng);
    for (int i = 0; i < popsize; i++)
    {   
        auto tmp = std::vector<double>();
        std::swap(tmp, m_individuals[i]->activation);
        m_individuals[i]->reset(rng);
        std::swap(tmp, m_individuals[i]->activation);
    }
    terminated = false;
    tab->reset();
    timer->tic();
    evaluate_population();
    this->iteration_geom = 1.0;
    end_iteration();
}


/*
 * Destructor function.
 */
CPopulation::~CPopulation()
{
    for (int i = 0; i < this->popsize; i++)
    {
        delete[] pop_info[i];
    }
    for (int i = 0; i < this->popsize; i++)
    {
        delete m_individuals[i];
    }
    
    m_individuals.clear();
    delete timer;
    timer=NULL;
    delete pt;
    pt=NULL;
    delete rng;
    rng=NULL;    
    delete tab;
    tab=NULL;
    delete[] pop_info;
    pop_info=NULL;
    delete[] permus;
    permus=NULL;
    delete[] genome_best;
    genome_best=NULL;
    delete[] templ_double_array;
    templ_double_array=NULL;
    delete[] templ_double_array2;
    templ_double_array2=NULL;
}



void CPopulation::end_iteration(){
    SortPopulation();
    get_population_info();
    iteration_geom *= iteration_geom_coef; // substitute of time, that way we do not depend on randomness
    if(timer->toc() > this->max_time_pso)
    {
        terminated = true;
    }
    //PrintMatrix(pop_info, this->popsize, NEAT::__sensor_N);
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
    double accept_or_reject_worse = output_neat[NEAT::accept_or_reject_worse];
    tab->tabu_coef_neat = output_neat[(int) NEAT::TABU];

    if
    (
        (-CUTOFF_0 < output_neat[0] && output_neat[0] < CUTOFF_0) ||
        (sum_abs_val_slice_vec(output_neat, 1, 1+NEAT::N_OPERATORS) == 0) 
    )
    {return;}


    else if(output_neat[0] < -CUTOFF_0){ // Local-search iteration.
        //#TODO check if unconnected output is 0.
        NEAT::operator_t operator_id = (NEAT::operator_t) argmax(output_neat + 1, NEAT::N_OPERATORS);
        this->problem->local_search_iteration(m_individuals[i], operator_id);
    }else if(output_neat[0] > CUTOFF_0){ // Move-with coeficients.
        NEAT::operator_t operator_id = (NEAT::operator_t) argmax(output_neat + 1, NEAT::N_OPERATORS);
        double* coef = output_neat + (NEAT::__output_N - NEAT::N_COEF);
        this->move_individual_i_based_on_coefs(coef, i, operator_id, accept_or_reject_worse);
        assert(isPermutation(this->m_individuals[i]->genome, this->n));

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
    comp_order_sparsity();
    //comp_r_number();
    load_local_opt();
}


void CPopulation::comp_relative_position()
{
    for (int i = 0; i < this->popsize; i++)
    {
        double res =  (double)i / (double)this->popsize;
        this->m_individuals[i]->relative_pos = res;
        pop_info[i][NEAT::RELATIVE_POSITION] = res;
    }
}

void CPopulation::comp_relative_time()
{
    for (int i = 0; i < this->popsize; i++)
    {
        double res = this->iteration_geom; // timer->toc() / MAX_TIME_PSO;
        this->m_individuals[i]->relative_time = res;
        pop_info[i][NEAT::RELATIVE_TIME] = res;
    }
    return ;
}

void CPopulation::comp_distance()
{

    // use the ranking of the differences in fitness with respect to the previous one. 
    templ_double_array[0] = DBL_MAX;
    for (int i = 1; i < this->popsize; i++)
    {   
        double val = m_individuals[i-1]->f_value -  m_individuals[i]->f_value;
        templ_double_array[i] = val;
    }

    compute_order_from_double_to_double(templ_double_array, this->popsize, templ_double_array2);


    // copy normalized values into individuals
    for (int i = 0; i < this->popsize; i++)
    {   
        double val = templ_double_array2[i] / (double) this->popsize;
        m_individuals[i]->distance = val;
        pop_info[i][NEAT::DISTANCE] = val;
    }

//region old_implementation_hamming_distance
    // // minimum of Hamming distance between the previous one and the next one 
    // // First, compute the distance of each permu with respect the  next permu
    // pop_info[0][NEAT::DISTANCE] = (double)Hamming_distance(m_individuals[0]->genome, m_individuals[1]->genome, n);
    // for (int i = 1; i < this->popsize - 1; i++)
    // {
    //     pop_info[i][NEAT::DISTANCE] = (double)Hamming_distance(m_individuals[i]->genome, m_individuals[i + 1]->genome, n);
    // }
    // pop_info[this->popsize - 1][NEAT::DISTANCE] = pop_info[this->popsize - 2][NEAT::DISTANCE];

    // // Then, assign to result_vector[i], the minimun of the distances between the next an the prev permus.
    // double distance_respect_to_previous = pop_info[0][NEAT::DISTANCE];
    // double temp;
    // for (int i = 1; i < this->popsize - 1; i++)
    // {
    //     temp = pop_info[i][NEAT::DISTANCE];
    //     pop_info[i][NEAT::DISTANCE] = MIN(pop_info[i][NEAT::DISTANCE], distance_respect_to_previous);
    //     distance_respect_to_previous = temp;
    // }

    // // Finally, normalize the values for them to be between 0 and 1.
    // for (int i = 0; i < this->popsize; i++)
    // {
    //     pop_info[i][NEAT::DISTANCE] /= (double)n;
    // }

    // // copy values into individuals
    // for (int i = 0; i < this->popsize; i++)
    // {
    //     m_individuals[i]->distance = pop_info[i][NEAT::DISTANCE];
    // }
//endregion old_implementation_hamming_distance

}


void CPopulation::comp_sparsity(){
    copy_references_of_genomes_from_individuals_to_permus();
    pt->compute_hamming_consensus(this->permus, this->popsize);
    for (int i = 0; i < this->popsize; i++)
    {
        m_individuals[i]->sparsity = 1.0 - pt->compute_normalized_hamming_distance_to_consensus(permus[i]);
        pop_info[i][NEAT::SPARSITY] = m_individuals[i]->sparsity;
    }
}


void CPopulation::comp_order_sparsity(){
    pt->compute_kendall_consensus_borda(this->permus, popsize);
    for (int i = 0; i < this->popsize; i++)
    {
        m_individuals[i]->order_sparsity = 1.0 - pt->compute_normalized_kendall_distance_to_consensus(permus[i]);
        pop_info[i][NEAT::ORDER_SPARSITY] = m_individuals[i]->order_sparsity;
    }
}


// void CPopulation::comp_r_number()
// {
//     for (int i = 0; i < this->popsize; i++)
//     {
//         double res =  rng->random_0_1_double();
//         pop_info[i][NEAT::R_NUMBER] = res;
//     }
// }

void CPopulation::load_local_opt(){
for (int i = 0; i < this->popsize; i++)
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
//         if (permutation_index != this->popsize - 1)
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
    for (int i = 0; i < this->popsize; i++)
    {
        permus[i] = m_individuals[i]->genome;
    }
}
 

void CPopulation::move_individual_i_based_on_coefs(double* coef_list, int i, NEAT::operator_t operator_id, double accept_or_reject_worse){

    int idx = pt->choose_permu_index_to_move(coef_list, rng);
    if (idx == -1){
        return;
    }


    assert(isPermutation(this->m_individuals[i]->genome, this->n)) ;

    bool towards = coef_list[idx] > 0;
    idx += (NEAT::__output_N - NEAT::N_COEF);

    int* ref_permu;
    
    assert(isPermutation(this->m_individuals[i]->genome, this->n)) ;


    switch (idx)
    {
    case NEAT::c_hamming_consensus:
        ref_permu = pt->hamming_mm_consensus;
        break;
    case NEAT::c_kendall_consensus:
        ref_permu = pt->kendall_mm_consensus;
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


    if(towards){
        problem->move_indiv_towards_reference(m_individuals[i], ref_permu, operator_id, accept_or_reject_worse);
        assert(isPermutation(this->m_individuals[i]->genome, this->n)) ;

    }else
    {
        problem->move_indiv_away_reference(m_individuals[i], ref_permu, operator_id, accept_or_reject_worse);
        assert(isPermutation(ref_permu, this->n)) ;
        assert(isPermutation(this->m_individuals[i]->genome, this->n)) ;

    }
}

}