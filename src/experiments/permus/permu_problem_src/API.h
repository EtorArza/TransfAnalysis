/*
 *  TSP.h
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 7/11/13.
 *  Copyright 2013 University of the Basque Country. All rights reserved.
 *
 */

#ifndef _API_H__
#define _API_H__

#include "PBP.h"
#include "Tools.h"
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
#include <stdio.h>
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
using namespace std;
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
using std::string;

class API : public PBP
{
	
public:

	/*
	 * The size of the problem.
	 */
	int m_size;

	/*
	 * The number of mallows models used in the instance.
	 */
	int m_num_mallows;
    
    /*
     * The list of consensus permutations.
     */
    int ** m_matrix_ConsensusPerms;
	
    /*
     * Theta values.
     */
    float * m_thetas;
    
    /*
     * Auxiliary array of values.
     */
    float * m_functionValues;
    
    /*
     * Auxiliary arrays for calculating Kendall distance.
     */
    int * m_v,*m_composition,*m_invertedB;
    
    /*
     * Auxiliary arrays for calculating Cayley distance.
     */
    int * m_elemsToCycles;
    int * m_maxPosInCycle;
    int * m_freeCycle;
    
    /*
     * Type of the neighborhood used to calculate distances.
     */
	int m_neighborhood;
    
    /*
     * Auxiliary vector.
     */
    int * m_aux2;
    /*
     * The constructor.
     */
	API();
	
    /*
     * The destructor.
     */
    virtual ~API();
	
	/*
	 * Read the instance file.
	 */
	int Read(string filename);
	
	/*
	 * This function evaluates the solution.
	 */
	float Evaluate(int * genes);


    
    /*
     * Returns the size of the problem.
     */
    int GetProblemSize();

    /*
     * Calculate distance between the permutation to the optimum.
     */
    int CalculateDistanceToOptimum(int*permutation);
    
private:
    /*
     * Calculate distance between two permutations.
     */
    int CalculateDistances(int*permutationA, int *permutationB);

    };
#endif
