/*
 *  LOP.cpp
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 11/21/11.
 *  Copyright 2011 University of the Basque Country. All rights reserved.
 *
 */

#include "LOP.h"
#include "Tools.h"
#include <assert.h>
#include "Individual.h"
/*
 * Class constructor.
 */
LOP::LOP()
{
}

LOP::~LOP()
{
	for (int i = 0; i < n; i++)
		delete[] m_matrix[i];
	delete[] m_matrix;
}

/*
 * Read LOP instance file.
 */
int LOP::Read(string filename)
{
	char line[2048]; // variable for input value
	string data = "";
	ifstream indata;
	indata.open(filename.c_str(), ios::in);
	int num = 0;
	while (!indata.eof())
	{

		indata.getline(line, 2048);
		stringstream ss;
		string sline;
		ss << line;
		ss >> sline;
		if (sline == "")
		{
			break;
		}
		if (num == 0)
		{
			n = atoi(line);
		}
		else
		{
			if (data == "")
				data = line;
			else
				data = data + ' ' + line;
		}
		num++;
	}
	indata.close();

	//BUILD MATRIX
	m_matrix = new double *[n];
	for (int i = 0; i < n; i++)
	{
		m_matrix[i] = new double[n];
	}

	istringstream iss(data);
	int i = 0;
	int j = 0;
	do
	{
		string sub;
		iss >> sub;
		if (sub != "")
		{
			//save distance in distances matrix.
			m_matrix[i][j] = atof(sub.c_str());
			if (j == (n - 1))
			{
				i++;
				j = 0;
			}
			else
			{
				j++;
			}
		}
		else
		{
			break;
		}
	} while (iss);

	initialize_variables_PBP(n);

	return (n);
}

double LOP::_Evaluate(int *permu)
{
	double fitness = 0;
	int i, j;
	for (i = 0; i < n - 1; i++)
		for (j = i + 1; j < n; j++)
			fitness += (double)m_matrix[permu[i]][permu[j]];
	return fitness;
}

void LOP::Evaluate(CIndividual *indiv)
{
	double fitness = 0;
	fitness = _Evaluate(indiv->genome);
	indiv->f_value = fitness;
}

/*
 * Returns the size of the problem.
 */
int LOP::GetProblemSize()
{
	return n;
}

// The Linear Ordering Problem: Instances, Search Space Analysis and Algorithms (Schiavinotto 2004)
double LOP::fitness_delta_swap(CIndividual *indiv, int i, int j)
{
	if(i==j){
		return 0;
	}

	else if (i == j + 1)
	{
		return m_matrix[indiv->genome[i]][indiv->genome[j]] - m_matrix[indiv->genome[j]][indiv->genome[i]];
	}
	else if (i == j - 1)
	{
		return m_matrix[indiv->genome[j]][indiv->genome[i]] - m_matrix[indiv->genome[i]][indiv->genome[j]];
	}
	else
	{
		std::cout << "error in function item_i_after_operator, return not reached";
		exit(1);
	}
}

/* 
 * SEARCH AND LEARNING FOR THE LINEAR ORDERING PROBLEM WITH AN APPLICATION TO MACHINE TRANSLATION (Roy Wesley Tromble 2009)
 * The local maxima of the interchange neighborhood are a superset of
 * the local maxima of Insertn, as Section 2.5.3 will argue. This neighborhood therefore
 * offers no computational advantage over Insertn, and this dissertation will not consider
 * it further 
 */
double LOP::fitness_delta_interchange(CIndividual *indiv, int i, int j)
{
	// the idea is to perform two insertion operations, assuming i < j. First insert item_i in position j, and then item_j in pos i
	if (i==j){
		return 0.0;
	}
	else if (i > j)
	{
		return fitness_delta_interchange(indiv, j, i);
	}
	else
	{	
		CIndividual *indiv_cop = indiv->Clone();
		apply_operator_with_fitness_update(indiv_cop, i, j, INSERT_OPERATOR);
		int f_value_cop = indiv_cop->f_value + fitness_delta_insert(indiv_cop, j-1, i);
		delete indiv_cop;
		return f_value_cop - indiv->f_value;
	}
}

double LOP::fitness_delta_insert(CIndividual *indiv, int i, int j)
{
	double delta = 0;
	if (i > j)
	{
		for (int k = j; k < i; k++)
		{
			delta += m_matrix[indiv->genome[i]][indiv->genome[k]] - m_matrix[indiv->genome[k]][indiv->genome[i]];
		}
	}
	else if (i < j)
	{
		for (int k = i + 1; k < j + 1; k++)
		{
			delta += m_matrix[indiv->genome[k]][indiv->genome[i]] - m_matrix[indiv->genome[i]][indiv->genome[k]];
		}
	}
	return delta;
}
