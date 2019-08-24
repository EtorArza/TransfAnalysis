//
//  Population.h
//  RankingEDAsCEC
//
//  Created by Josu Ceberio Uribe on 11/19/13.
//  Copyright (c) 2013 Josu Ceberio Uribe. All rights reserved.
//

#ifndef POPULATION_H_
#define POPULATION_H_

#include <vector>
#include <algorithm>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "Individual.h"
#include "PBP.h"
#include "Tools.h"

using std::istream;
using std::ostream;
using std::string;
using std::stringstream;
using namespace std;

class CPopulation
{

public:
   struct Better
   {
      bool operator()(CIndividual *a, CIndividual *b) const
      {
         return a->f_value > b->f_value;
      }
   } Better;

   /*
     * Vector of individuals that constitute the population.
     */
   vector<CIndividual *> m_individuals;

   /* 
     * Size of the vector of individuals
     */
   int n;

   /*
     * Size of the population
     */
   int popsize;

   /*
     * Population info. Information about each individual. 
     * popinfo[i][j] has information about propertie i from individual j.
     */
   double **pop_info;

   /*
     * The constructor. It creates an empty list.
     */
   CPopulation(int pop_size, int individual_size);

   /*
     * The destructor.
     */
   virtual ~CPopulation();

   /*
     * Sorts the individuals in the population in decreasing order of the fitness value.
     */
   void SortPopulation(void);

   bool Exists(int *permutation, int problem_size, int sel_size);

   /*
	 * Prints the current population.
	 */
   void Print();



   /*
     * Calculates the average fitness of the first 'size' solutions in the population
     */
   double AverageFitnessPopulation(int size);

   // evaluate the whole population
   void evaluate_population(PBP *problem);

   // Fill pop_info. Assumes the fitness values are computed, and that population is sorted.
   void get_population_info();

   void take_action_with_action_id(int permutation_index, int action_id, PBP *problem);

private:
   void comp_relative_position();
   void comp_relative_time();
   void comp_distance();
   void comp_sparsity();

   PermuTools *pt;
   double relative_time();
   stopwatch *timer;

   /* 
  * In this case, 0 means highly cramped, 1 means highly sparse.
  * Each permutation is compared with the next and previous permutations.
  * For example, if permutations on position 5 and 6 are the same, then result_vector[5] = result_vector[6] = 0
  */
   void get_info_Hamming_distance_from_adjacent_permus(double *result_vector);
   int **permus; //this is only intended to temporally store the permutations for computations.
   void copy_references_of_genomes_from_individuals_to_permus();
};

#endif