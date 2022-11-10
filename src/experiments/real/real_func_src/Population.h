#pragma once


#include <string.h>
#include "Individual.h"
#include "../REAL_FUNC_params.h"
class MultidimBenchmarkFF;
class RandomNumberGenerator;


class CPopulation
{

public:
  // if no rng object is provided, a new one is generated
  CPopulation(MultidimBenchmarkFF *problem, REAL_FUNC::params* parameters);
  CPopulation(MultidimBenchmarkFF *problem, RandomNumberGenerator* rng, REAL_FUNC::params* parameters);
  void init_class(MultidimBenchmarkFF *problem, RandomNumberGenerator* rng, REAL_FUNC::params* parameters);


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
  long max_evals;

  bool full_model = false;
  double f_best;
	double* genome_best;

  void Print();
  void print_positions(std::string filename);
  void end_iteration(); // sort the population, check if the best solution was improved, and coompute neat imputs.
  void Reset();

  double* get_neat_input_individual_i(int i);
  void apply_neat_output_to_individual_i(double* output_neat, int i);
  bool terminated;

   /*
   * Population info. Information about each individual. 
   * popinfo[i][j] has information about propertie i from individual j.
   */
  double **pop_info;
  stopwatch *timer;
  RandomNumberGenerator *rng;

private:



  // evaluate the whole population. Only used to initialize the population.
  void evaluate_population();

  void SortPopulation(void);
  // Fill pop_info. Assumes the fitness values are computed, and that population is sorted.
  void get_population_info();


  void comp_relative_score();
  void comp_relative_time();
  void comp_distance_to_closest();
  void comp_RELATIVE_DIST_TO_AVERAGE();
  void comp_relative_dist_to_best();
  void  load_INDIVIDUAL_BEST_WAS_IMPROVED();
  void  load_GLOBAL_BEST_WAS_IMPROVED();
  void load_RANDOM_NUMBER();


  MultidimBenchmarkFF * problem;
  double *average_solution_in_population;
  double *templ_double_array0_of_size_n;
  double *templ_double_array1_of_size_n;
  double *templ_double_array2_of_size_n;
  double *templ_double_array3_of_size_n;
  double *templ_double_array4_of_size_n;
  double *templ_double_array1_of_size_POPSIZE;
  double *templ_double_array2_of_size_POPSIZE;
  double relative_time();

  int n_evals;
  bool global_best_was_improved = false;

  /* 
  * In this case, 0 means highly cramped, 1 means highly sparse.
  * Each permutation is compared with the next and previous permutations.
  * For example, if permutations on position 5 and 6 are the same, then result_vector[5] = result_vector[6] = 0
  */
  void get_info_Hamming_distance_from_adjacent_permus(double *result_vector);

  // copy the reference of the permutations in the individuals to a matrix of permutations for analisys in sparsity and distances.
  void copy_references_of_genomes_from_individuals_to_permus();
};

