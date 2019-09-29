//
//  Individual.cc
//  RankingEDAsCEC
//
//  Created by Josu Ceberio Uribe on 11/19/13.
//  Copyright (c) 2013 Josu Ceberio Uribe. All rights reserved.
//

#include "Individual.h"
#include "Tools.h"
#include <limits>
#include "Parameters.h"
#include <iomanip>      // std::setprecision
#include "../permuevaluator.h"


// The object storing the values of all the individuals
// that have been created during the execution of the
// program. 

int CIndividual::n_indivs_created = 0;


CIndividual::CIndividual(int length, RandomNumberGenerator* rng)
{
	n = length;
	genome = new int[n];
	momentum = new int[n];
	genome_best = new int[n];
	GenerateRandomPermutation(this->genome, n, rng); 
	GenerateRandomPermutation(this->momentum, n, rng); 
	GenerateRandomPermutation(this->genome_best, n, rng);
	f_value=std::numeric_limits<double>::lowest();
	f_best=std::numeric_limits<double>::lowest();
	id = n_indivs_created;
	n_indivs_created++;

}

void CIndividual::reset(RandomNumberGenerator* rng){
	GenerateRandomPermutation(this->genome, n, rng); 
	GenerateRandomPermutation(this->momentum, n, rng); 
	GenerateRandomPermutation(this->genome_best, n, rng);
	f_value=std::numeric_limits<double>::lowest();
	f_best=std::numeric_limits<double>::lowest();
}

CIndividual::~CIndividual()
{
	delete[] this->genome;
	delete[] this->momentum;
	delete[] this->genome_best;
	genome=NULL;
}



/*
 * Output operator.
 */
ostream & operator<<(ostream & os,CIndividual * & individual)
{
	// #define RELATIVE_POSITION 0
	// #define RELATIVE_TIME 1
	// #define DISTANCE 2
	// #define SPARSITY 3
	std::ios oldState(nullptr); //save state of std::cout to restore it after changes.
	oldState.copyfmt(std::cout);
	os << std::fixed;
	os << std::setprecision(0);
	os << std::setfill(' ');
	os << std::setw(7);
	os <<individual->f_value<<" - "; 
	os << std::setprecision(3);
	os << std::setw(5);
	os <<individual->relative_pos<<" - "; 
	os <<individual->relative_time<<" - "; 
	os <<individual->distance<<" - ";
	os <<individual->sparsity<<" - "; 
	os << std::setw(0);
	os << "[";
	os << std::setw(2);
	os << std::setprecision(0);
	os << std::fixed;
	os << std::setfill(' ');
    os << (double) individual->genome[0];
	for(int i=1;i<individual->n;i++){
		os << std::setw(0);
		os << ", ";
		os << std::setprecision(0);
		os << std::fixed;
		os << std::setfill(' ');
		os << std::setw(2);

        os << (double) individual->genome[i];
	}
	os << std::setw(0);
	os << "]";
	std::cout.copyfmt(oldState); // restore state of std::cout
	return os;
}

/*
 * Input operator.
 */
istream & operator>>(istream & is,CIndividual * & individual)
{
  char k; //to avoid intermediate characters such as ,.

  is >> individual->f_value;
  is >> individual->genome[0];
  for(int i=1;i<individual->n;i++)
    is >> k >> individual->genome[i];
  is >> k;

  return is;
}


/*
 * Sets the given array of ints as the genes of the individual.
 */
void CIndividual::SetGenome(int * genome_to_be_placed_in_individual)
{
	memcpy(this->genome, genome_to_be_placed_in_individual, sizeof(int)*n);
	f_value=MIN_LONG_INTEGER;
}

/*
 * Prints in the standard output genes.
 */
void CIndividual::PrintGenome()
{	cout << "[";
	for (int i=0;i<n;i++){
		cout<<genome[i]<<" ";
	}
	cout<<"]"<<endl;
}

/*
 * Clones the individual.
 */
CIndividual * CIndividual::Clone()
{
	RandomNumberGenerator tmp_rng;
	CIndividual * ind = new CIndividual(n, &tmp_rng);
	ind->SetGenome(genome);
	ind->f_value = this->f_value;
	return ind;
}



