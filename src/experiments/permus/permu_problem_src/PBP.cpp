/*
 *  PBP.cpp
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 7/11/13.
 *  Copyright 2013 University of the Basque Country. All rights reserved.
 *
 */

#include "PBP.h"
#include "Tools.h"
#include "Individual.h"
#include "Parameters.h"
#include <assert.h>
#include "../permuevaluator.h"
#include "Tabu.h"
#include <limits.h>


PBP::PBP()
{
    rng = new RandomNumberGenerator();
}

PBP::~PBP()
{
    delete[] _random_permu1;
    delete[] _random_permu2;
    delete[] _random_permu3;
    delete rng;
}
// This function needs to be called when the read procedure is called
void PBP::initialize_variables_PBP(int problem_size)
{   
    if (problem_size < 2)
    {
        std::cout << "error, problem size < 2. Instance file not read correctly." << std::flush;
        exit(1);
    }
    
    _random_permu1 = new int[problem_size];
    GenerateRandomPermutation(_random_permu1, problem_size,rng);
    _random_permu2 = new int[problem_size];
    GenerateRandomPermutation(_random_permu2, problem_size,rng);
    _random_permu3 = new int[problem_size];
    GenerateRandomPermutation(_random_permu3, problem_size,rng);
    problem_size_PBP = problem_size;
}

void PBP::Evaluate(CIndividual *indiv)
{
	double fitness = 0;
	fitness = _Evaluate(indiv->genome);
	indiv->f_value = fitness;
}

double PBP::Evaluate(int *genome)
{
	return _Evaluate(genome);
}

std::mutex PBP::mut;
int PBP::Read_with_mutex(string filename){
    PBP::mut.lock();
    int res = this->Read(filename);
    PBP::mut.unlock();
    return res;
}


void PBP::apply_operator_with_fitness_update(CIndividual *indiv, int i, int j, NEAT::operator_t operator_id, double accept_or_reject_worse)
{   
    if(i==j && j==-1){
        return;
    }
    double delta = 0;


    switch (operator_id)
    {
    case NEAT::SWAP:
        delta = this->fitness_delta_swap(indiv, i, j);
        break;
    case NEAT::EXCH:
        delta = this->fitness_delta_interchange(indiv, i, j);
        break;
    case NEAT::INSERT:
        delta = this->fitness_delta_insert(indiv, i, j);
        break;

    default:

        std::cout << "operator_id not recognized.";
        exit(1);
        break;
    }

    apply_operator_with_fitness_update(indiv, delta, i, j, operator_id, accept_or_reject_worse);

}

void PBP::apply_operator_with_fitness_update(CIndividual *indiv, double delta, int i, int j, NEAT::operator_t operator_id, double accept_or_reject_worse)
{
    if (i == j && j == -1)
    {
        return;
    }


    double r = 2 * (this->rng->random_0_1_double() - 0.5001);

    bool reject_worse = false;
    bool moved = false;

    if (r > accept_or_reject_worse)
    {
        reject_worse = true;
    }

    if (delta > 0 || !reject_worse)
    {
        indiv->f_value = delta + indiv->f_value;

        switch (operator_id)
        {
        case NEAT::SWAP:
            Swap(indiv->genome, i, j);
            break;

        case NEAT::EXCH:
            Swap(indiv->genome, i, j);
            break;

        case NEAT::INSERT:
            InsertAt(indiv->genome, i, j, this->problem_size_PBP);
            break;

        default:
            std::cout << "operator_id not recognized.";
            exit(1);
            break;
        }

        //double actual_delta = indiv->f_value - Evaluate(indiv->genome);
        assert(abs(indiv->f_value - Evaluate(indiv->genome)) < 0.0001);
        moved = true;
    }

    if (indiv->f_value > indiv->f_best)
    {
        indiv->f_best = indiv->f_value;
        copy_vector(indiv->genome_best, indiv->genome, this->GetProblemSize());
    }

    if (moved)
    {
        for (int i = 0; i < 3; i++)
        {
            indiv->is_local_optimum[i] = false;
        }
    }
}




void PBP::local_search_iteration(CIndividual *indiv, NEAT::operator_t operator_id)
{
    if (indiv->is_local_optimum[operator_id])
    {
        return;
    }



    switch (operator_id)
    {
    case NEAT::SWAP:
        shuffle_vector(_random_permu1, problem_size_PBP, rng);
        for (int i = 0; i < problem_size_PBP; i++)
        {
            int r = _random_permu1[i];
            if (r == problem_size_PBP - 1)
            {
                continue;
            }
            if (tab->is_tabu(r, r+1))
            {
                continue;
            }
            
            double delta = fitness_delta_swap(indiv, r, r+1);
            if (delta > 0)
            {
                tab->set_tabu(r, r+1);
                apply_operator_with_fitness_update(indiv, delta, r, r+1, operator_id);
                assert(abs(indiv->f_value - _Evaluate(indiv->genome)) < 0.0001);
                return;
            }
        }
        break;

    case NEAT::EXCH:
        shuffle_vector(_random_permu1, this->problem_size_PBP, rng);
        shuffle_vector(_random_permu2, this->problem_size_PBP, rng);
        for (int i = 0; i < this->problem_size_PBP; i++)
        {
            for (int j = 0; j < this->problem_size_PBP; j++)
            {
                if (i < j && !(tab->is_tabu(_random_permu1[i],_random_permu2[j]) || tab->is_tabu(_random_permu2[j],_random_permu1[i])) )
                {
                    double delta = fitness_delta_interchange(indiv, _random_permu1[i], _random_permu2[j]);
                    if (delta > 0)
                    {
                       	//cout << "(" << _random_permu1[i] << "," << _random_permu2[j] << ")" << endl;
                        tab->set_tabu(_random_permu1[i],_random_permu2[j]);
                        assert(isPermutation(indiv->genome, problem_size_PBP));
                        apply_operator_with_fitness_update(indiv, delta, _random_permu1[i], _random_permu2[j], operator_id);
                        assert(isPermutation(indiv->genome, problem_size_PBP));
                        assert(abs(indiv->f_value - _Evaluate(indiv->genome)) < 0.0001);
                        return;
                    }
                }
            }
        }
        break;

    case NEAT::INSERT:
        shuffle_vector(_random_permu1, this->problem_size_PBP,rng);
        shuffle_vector(_random_permu2, this->problem_size_PBP,rng);
        for (int i = 0; i < this->problem_size_PBP; i++)
        {
            for (int j = 0; j < this->problem_size_PBP; j++)
            {
                if (i != j || (i > 0 &&_random_permu1[j]==_random_permu1[i-1]))
                {
                    if (tab->is_tabu(_random_permu1[i], _random_permu2[j]))
                    {
                        continue;
                    }
                    double delta =fitness_delta_insert(indiv, _random_permu1[i], _random_permu2[j]);
                    if (delta > 0)
                    {   
                        //cout << "peep ";
                        tab->set_tabu(_random_permu1[i], _random_permu2[j]);
                       	//cout << "(" << i << "," << j << ")" << endl;
                        apply_operator_with_fitness_update(indiv, delta, _random_permu1[i], _random_permu2[j], operator_id);

                        // if(!isPermutation(indiv->genome, problem_size_PBP)){
                        //     cout << "is NOT permutation" << endl;
                        //     exit(1);
                        // }
                        //cout << f - indiv->f_value;
                        return;
                    }
                }
            }
        }
        break;

    default:
        std::cout << "operator_id not recognized.";
        exit(1);
        break;
    }

    indiv->is_local_optimum[operator_id] = true;
    if (operator_id == NEAT::EXCH)
    {
        indiv->is_local_optimum[NEAT::OPT_SWAP] = true;
    }



}

// obtain item at pos idx assuming operator operator_id where applied.
int PBP::item_i_after_operator(int *permu, int idx, NEAT::operator_t operator_id, int i, int j){
    switch (operator_id)
    {
    case NEAT::SWAP:
        if (idx != i && idx != j)
        {
            return permu[idx];
        }else if(idx == i)
        {
            return permu[j];
        }else
        {
            return permu[i];
        }
        std::cout <<  "idx not valid";
        exit(1);
        break;
    case NEAT::EXCH:
        if (idx != i && idx != j)
        {
            return permu[idx];
        }else if(idx == i)
        {
            return permu[j];
        }else
        {
            return permu[i];
        }
        std::cout << "idx not valid";
        exit(1);
        break;

    case NEAT::INSERT:
        if(i < j){
            if (idx == j)
            {
                return permu[i];
            }else if(idx >= i && idx < j){
                return permu[idx + 1];
            }else
            {
                return permu[idx];
            }
        }else if( i > j){
            if (idx == j)
            {
                return permu[i];
            }else if(idx >= j+1 && idx < i+1){
                return permu[idx - 1];
            }else
            {
                return permu[idx];
            }
        }else{ // i == j
            return permu[idx];
        }
        std::cout << "error in function item_i_after_operator, return not reached";
        exit(1);
        break;

    default:
        std::cout << "operator_id not recognized.";
        exit(1);
        break;
    }
}


void PBP::obtain_indexes_step_towards(int *permu, int *ref_permu, int* i, int* j, NEAT::operator_t operator_id)
{
    switch (operator_id)
    {
    case NEAT::SWAP:  // SCHIZVINOTTO 2007
    {

        // compute inverse (position) of permu
        for (int i = 0; i < problem_size_PBP; i++)
        {
            _random_permu2[permu[i]] = i;
        }

        for (int idx = 0; idx < problem_size_PBP; idx++) // compute permu^-1 \circ ref_permu  
        {
            _random_permu1[idx] = _random_permu2[ref_permu[idx]];
        }

        GenerateRandomPermutation(_random_permu2, problem_size_PBP,rng);

        // compute inverse (position) of permu^-1 \circ ref_permu  
        for (int i = 0; i < problem_size_PBP; i++)
        {
            _random_permu3[_random_permu1[i]] = i;
        }

        for (int idx = 0; idx < problem_size_PBP; idx++)
        {
            int r = _random_permu2[idx];
            if ((r < problem_size_PBP - 1 && _random_permu3[r] > _random_permu3[r+1])){
                *i = r;
                *j = r+1;
                return;
            }
        }
        *i = -1;
        *j = -1;
        return;

        break;
    }
    case NEAT::EXCH:  // SCHIZVINOTTO 2007
    {
        for (int idx = 0; idx < problem_size_PBP; idx++) // compute ref_permu \circ permu^-1
        {   
            _random_permu1[idx] = Find(permu, problem_size_PBP, ref_permu[idx]);
        }
        shuffle_vector(_random_permu2, problem_size_PBP,rng);
        
        for (int idx = 0; idx < problem_size_PBP; idx++)
        {
            int r = _random_permu2[idx];
            if (_random_permu1[r] != r){
                *i = r;
                *j = _random_permu1[r];
                return;
            }
        }
        
        // permu == ref_permu

        *i = -1;
        *j = -1;
        return;

    }
    case NEAT::INSERT:
        // get indices to insert considering the item that makes the biggest "jump"    
    {

        // compute inverse (position) of permu
        for (int i = 0; i < problem_size_PBP; i++)
        {
            _random_permu2[permu[i]] = i;
        }

        for (int idx = 0; idx < problem_size_PBP; idx++) // compute permu^-1 \circ ref_permu  
        {
            _random_permu1[idx] = _random_permu2[ref_permu[idx]];
        }


        // compute inverse (position) of permu^-1 \circ ref_permu  
        for (int i = 0; i < problem_size_PBP; i++)
        {
            _random_permu3[_random_permu1[i]] = i;
        }

        GenerateRandomPermutation(_random_permu2, problem_size_PBP,rng);
        int max = 0;
        int arg_max = 0;

        for (int idx = 0; idx < problem_size_PBP; idx++) // compute ref_permu \circ permu^-1
        {   
            int r = _random_permu2[idx];
            if (abs(_random_permu3[r] - r) > max)
            {
                max = abs(_random_permu3[r] - r);
                arg_max = r;
            }
        }
        if (max == 0)
        {
            *i = -1;
            *j = -1;
        }else{
            *i = arg_max;
            *j = _random_permu3[*i];
        }
        break;
    }
    default:
        std::cout << "operator_id not recognized.";
        exit(1);
        break;
    }
    assert(isPermutation(permu, this->problem_size_PBP));
}

void PBP::obtain_indexes_step_away(int *permu, int *ref_permu, int* i, int* j, NEAT::operator_t operator_id)
{
    switch (operator_id)
    {
    case NEAT::SWAP:
    {
        // compute inverse (position) of permu
        for (int i = 0; i < problem_size_PBP; i++)
        {
            _random_permu2[permu[i]] = i;
        }

        for (int idx = 0; idx < problem_size_PBP; idx++) // compute permu^-1 \circ ref_permu  
        {
            _random_permu1[idx] = _random_permu2[ref_permu[idx]];
        }

        GenerateRandomPermutation(_random_permu2, problem_size_PBP,rng);

        // compute inverse (position) of permu^-1 \circ ref_permu
        for (int i = 0; i < problem_size_PBP; i++)
        {
            _random_permu3[_random_permu1[i]] = i;
        }

        for (int idx = 0; idx < problem_size_PBP; idx++)
        {
            int r = _random_permu2[idx];
            if ((r < problem_size_PBP - 1 && _random_permu3[r] < _random_permu3[r + 1]))
            { // change this line from > to <
                *i = r;
                *j = r + 1;
                return;
            }
        }
        *i = -1;
        *j = -1;
        return;

        break;
    }
    case NEAT::EXCH:
    {
        for (int idx = 0; idx < problem_size_PBP; idx++)
        {   
            _random_permu1[idx] = Find(permu, problem_size_PBP, ref_permu[idx]);
        }
        shuffle_vector(_random_permu2, problem_size_PBP,rng);
        
        for (int idx = 0; idx < problem_size_PBP; idx++)
        {
            int r = _random_permu2[idx];
            if (_random_permu1[r] == r){ // change != with ==
                *i = r;
                *j = _random_permu1[r];
                return;
            }
        }
        
        // permu == ref_permu

        *i = -1;
        *j = -1;
        return;

    }
    case NEAT::INSERT:
    {
        assert(isPermutation(permu, problem_size_PBP)) ;
        assert(isPermutation(ref_permu, problem_size_PBP)) ;
        assert(isPermutation(_random_permu1, problem_size_PBP)) ;
        assert(isPermutation(_random_permu2, problem_size_PBP)) ;
        assert(isPermutation(_random_permu3, problem_size_PBP)) ;

        // compute inverse (position) of permu
        for (int i = 0; i < problem_size_PBP; i++)
        {
            _random_permu2[permu[i]] = i;
        }

        assert(isPermutation(_random_permu2, problem_size_PBP)) ;

        for (int idx = 0; idx < problem_size_PBP; idx++) // compute permu^-1 \circ ref_permu  
        {
            _random_permu1[idx] = _random_permu2[ref_permu[idx]];
        }

        assert(isPermutation(_random_permu1, problem_size_PBP)) ;

        // compute inverse (position) of permu^-1 \circ ref_permu  
        for (int i = 0; i < problem_size_PBP; i++)
        {
            _random_permu3[_random_permu1[i]] = i;
        }

        assert(isPermutation(_random_permu3, problem_size_PBP)) ;


        GenerateRandomPermutation(_random_permu2, problem_size_PBP,rng);
        assert(isPermutation(_random_permu2, problem_size_PBP)) ;

        int max = __INT_MAX__;
        int arg_max = 0;

        for (int idx = 0; idx < problem_size_PBP; idx++) 
        {
            int r = _random_permu2[idx];
            if (abs(_random_permu3[r] - r) < max) // change to find minimum
            {
                max = abs(_random_permu3[r] - r);
                arg_max = r;
            }
        }
        *i = arg_max;
        *j = rng->random_integer_uniform(problem_size_PBP);
        assert(*j < problem_size_PBP);
        while (*i ==*j)
        {
            *j = rng->random_integer_uniform(problem_size_PBP);
        }
        
        break;
    }
    default:
        std::cout << "operator_id not recognized.";
        exit(1);
        break;
    }

}


void PBP::move_indiv_towards_reference(CIndividual* indiv, int* ref_permu, NEAT::operator_t operator_id, double accept_or_reject_worse){
    int i,j;
    obtain_indexes_step_towards(indiv->genome, ref_permu, &i, &j, operator_id);
    if (tab->is_tabu(i,j))
    {
        return;
    }else
    {
        tab->set_tabu(i,j);
    }
    // std::cout << "(" << i << "," << j << ")" << endl;
    apply_operator_with_fitness_update(indiv, i, j, operator_id, accept_or_reject_worse);
    assert(abs(indiv->f_value - _Evaluate(indiv->genome)) < 0.0001);

}


void PBP::move_indiv_away_reference(CIndividual* indiv, int* ref_permu, NEAT::operator_t operator_id, double accept_or_reject_worse){
    int i,j;
    obtain_indexes_step_away(indiv->genome, ref_permu, &i, &j, operator_id);
    // std::cout << "(" << i << "," << j << ")" << endl;

    if (tab->is_tabu(i,j))
    {
        return;
    }else
    {
        tab->set_tabu(i,j);
    }

    apply_operator_with_fitness_update(indiv, i, j, operator_id, accept_or_reject_worse);
    assert(abs(indiv->f_value - _Evaluate(indiv->genome)) < 0.0001);
    assert(isPermutation(indiv->genome, this->problem_size_PBP)) ;

}