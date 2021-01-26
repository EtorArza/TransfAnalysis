//
//  Individual.h
//  RankingEDAsCEC
//
//  Created by Josu Ceberio Uribe on 11/19/13.
//  Copyright (c) 2013 Josu Ceberio Uribe. All rights reserved.
//

#pragma once


#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <limits.h>
#include <iostream>
#include "Tools.h"
#include "../real_func_evaluator.h"


using namespace std;
using std::istream;
using std::ostream;

namespace NEAT{
class CpuNetwork;
}
class CIndividual
{
public:
	// The constructor. The constructed individual has
	// all zeroes as its genes.
	CIndividual(int length, RandomNumberGenerator* rng);
	void reset(RandomNumberGenerator* rng);
	~CIndividual();
	void SetGenome(double *genes);
	void PrintGenome();
	CIndividual *Clone();

	/*
     * Output operator.
     */
	friend ostream &operator<<(ostream &os, CIndividual *&individual);

	/*
     * Input operator.
     */
	friend istream &operator>>(istream &is, CIndividual *&individual);

	int n;
	double *genome; //The genes of the individual, or the permutation.
	int id;		 // a unique identifier for each individual in the pop.
	double f_value;
	double f_best;
	double* genome_best;
	double* momentum;

	// pop_info
	double relative_pos = 0;
	double relative_time = 0;
	double sparsity = 0;
	double order_sparsity = 0;
	double distance = 0;
	
	std::vector<double> activation;

	bool bk_was_improved = false;

private:
	static int n_indivs_created;
};


