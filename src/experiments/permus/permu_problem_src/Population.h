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
  CPopulation(PBP *problem);
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

 	double f_best;
	int* genome_best;


  void Print();
  void end_iteration(); // sort the population, check if the best solution was improved, and coompute neat imputs.


  double* get_neat_input_individual_i(int i);
  void apply_neat_output_to_individual_i(double* output_neat, int i);

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
  double relative_time();
  stopwatch *timer;
   /*
   * Population info. Information about each individual. 
   * popinfo[i][j] has information about propertie i from individual j.
   */
  double **pop_info;


  /* 
  * In this case, 0 means highly cramped, 1 means highly sparse.
  * Each permutation is compared with the next and previous permutations.
  * For example, if permutations on position 5 and 6 are the same, then result_vector[5] = result_vector[6] = 0
  */
  void get_info_Hamming_distance_from_adjacent_permus(double *result_vector);
  int **permus; //this is only intended to temporally store the permutations for computations.

  // copy the reference of the permutations in the individuals to a matrix of permutations for analisys in sparsity and distances.
  void copy_references_of_genomes_from_individuals_to_permus();
};

