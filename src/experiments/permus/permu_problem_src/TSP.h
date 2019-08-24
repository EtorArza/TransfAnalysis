/*
 *  TSP.h
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 7/11/13.
 *  Copyright 2013 University of the Basque Country. All rights reserved.
 *
 */

#ifndef _TSP_H__
#define _TSP_H__

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

class TSP : public PBP
{
	
public:
	
    /*
     * Matrix of distances between the cities.
     */
	double ** m_distance_matrix;
	
	/*
	 * The number of cities.
	 */
	int m_size;
	
	/*
     * The constructor.
     */
	TSP();
	
    /*
     * The destructor.
     */
    virtual ~TSP();
	
	/*
	 * Read TSP instance file that belongs to the TSPLIB library.
	 */
	int Read2(string filename);

    /*
	 * Read TSP instance file.
	 */
	int Read(string filename);
    
	/*
	 * This function evaluates the fitness of the solution for the TSP problem.
	 */
	double Evaluate(int * genes);

  
    /*
     * Returns the size of the problem.
     */
    int GetProblemSize();
private:
	
};
#endif
