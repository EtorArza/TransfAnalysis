//
//  Population.h
//  RankingEDAsCEC
//
//  Created by Josu Ceberio Uribe on 11/19/13.
//  Copyright (c) 2013 Josu Ceberio Uribe. All rights reserved.
//

#pragma once


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
  // if no rng object is provided, a new one is generated
  CPopulation(PBP *problem);
  CPopulation(PBP *problem, RandomNumberGenerator* rng);
  void init_class(PBP *problem, RandomNumberGenerator* rng);


  virtual ~CPopulation();

  struct Better
  {
    bool operator()(CIndividual *a, CIndividual *b) const
    {
      return a->f_value > b->f_value;
    }
  } Better;

  // Vector of individuals that constitute the population.
  vector<CIndividual *> m_individuals;
  int n;
  int popsize;

 	float f_best;
	int* genome_best;


  void Print();
  void end_iteration(); // sort the population, check if the best solution was improved, and coompute neat imputs.
  void Reset();

  float* get_neat_input_individual_i(int i);
  void apply_neat_output_to_individual_i(float* output_neat, int i);
  bool terminated;

   /*
   * Population info. Information about each individual. 
   * popinfo[i][j] has information about propertie i from individual j.
   */
  float **pop_info;
  stopwatch *timer;
  RandomNumberGenerator *rng;

private:

  // evaluate the whole population. Only used to initialize the population.
  void evaluate_population();

  void SortPopulation(void);
  // Fill pop_info. Assumes the fitness values are computed, and that population is sorted.
  void get_population_info();


  void comp_relative_position();
  void comp_relative_time();
  void comp_distance();
  void comp_sparsity();
  void comp_r_number();
  void load_local_opt();

  PBP * problem;
  PermuTools *pt;
  float relative_time();



  /* 
  * In this case, 0 means highly cramped, 1 means highly sparse.
  * Each permutation is compared with the next and previous permutations.
  * For example, if permutations on position 5 and 6 are the same, then result_vector[5] = result_vector[6] = 0
  */
  void get_info_Hamming_distance_from_adjacent_permus(float *result_vector);
  int **permus; //this is only intended to temporally store the permutations for computations.

  // copy the reference of the permutations in the individuals to a matrix of permutations for analisys in sparsity and distances.
  void copy_references_of_genomes_from_individuals_to_permus();

  // move permutation based on coefs. and other permus.
  // first choose a permu proportionally to its weight. 
  // Then if weight is positive move towards, otherwise, move away from.
  // to permuevaluator.h contains which permutations are considered.
  void move_individual_i_based_on_coefs(float* coef_list, int i, int operator_id);


 
};

