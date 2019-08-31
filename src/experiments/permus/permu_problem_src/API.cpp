/*
 *  TSP.cpp
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 7/11/13.
 *  Copyright 2013 University of the Basque Country. All rights reserved.
 *
 */

#include "API.h"
#include <stdio.h>
#include <sys/time.h>
/*
 *Class constructor.
 */
API::API()
{
	m_size=0;
    m_num_mallows=0;
    m_neighborhood=0;
    
}

/*
 * Class destructor.
 */
API::~API()
{
	delete [] m_aux2;
    delete [] m_v;
    delete [] m_composition;
    delete [] m_invertedB;
}


/*
 * Read the instance file.
 */
int API::Read(string filename)
{
    int i,j,func;

    FILE * instance_file;
    
    instance_file=fopen(filename.c_str(),"r");
    
    fscanf(instance_file, "%d %d %d %d",&m_size,&m_num_mallows,&func,&m_neighborhood);
    
    m_thetas=new float[m_num_mallows];
    m_functionValues=new float[m_num_mallows];
    m_matrix_ConsensusPerms=new int*[m_num_mallows];
    for (i=0; i<m_num_mallows; i++) {
        m_matrix_ConsensusPerms[i]=new int[m_size];
    }

    m_composition=new int[m_size];
    m_invertedB=new int[m_size];
    if (m_neighborhood==1){
        m_aux2=new int [m_size];
        m_v=new int[m_size-1];
    }
    else if (m_neighborhood==2){
        m_elemsToCycles=new int[m_size];
        m_maxPosInCycle=new int[m_size];
        m_freeCycle=new int[m_size];
    }
        
        
    for (i=0; i<m_num_mallows; i++) {
        for (j=0; j<m_size; j++) {
            fscanf(instance_file, "%d ",&m_matrix_ConsensusPerms[i][j]);
        }
        fscanf(instance_file, "%lf ",&m_functionValues[i]);
        m_functionValues[i]=log(m_functionValues[i]);
        fscanf(instance_file, "%lf ",&m_thetas[i]);
    }
    
    fclose(instance_file);


   	initialize_variables_PBP(m_size);


    return (m_size);

}


/*
 * This function evaluates the fitness of the solution.
 */

float API::Evaluate(int * genes)
{
    
    int dist,maxind,stop,pos;
    float thetamax,maxf,thetapos,maxpos;
    dist=CalculateDistances(genes, m_matrix_ConsensusPerms[0]);

    maxind=0;
    thetamax=m_thetas[maxind]*dist;
    maxf=m_functionValues[maxind]-thetamax;
    stop=0;
    pos=1;
    
    while (!stop && pos<m_num_mallows) {
        if (maxf > m_functionValues[pos]){
            stop=1;
        }
        else {
            dist=CalculateDistances(genes, m_matrix_ConsensusPerms[pos]);
            thetapos=m_thetas[pos]*dist;
            maxpos=m_functionValues[pos]-thetapos;
            
            if (thetapos<thetamax && maxf < maxpos){
                maxind=pos;
                maxf=maxpos;
            }
            pos++;
        }
    }
    return(maxf);
}



/*
 * Returns the size of the problem.
 */
int API::GetProblemSize()
{
    return m_size;
}

/*
 * Calculates the distance between two permutations.
 */
int API::CalculateDistances(int*permutationA, int *permutationB){
    int dist=0;
    switch (m_neighborhood) {
        case 1:
            dist=Kendall(permutationA,permutationB, m_size, m_aux2, m_invertedB,m_composition,m_v);
            break;
        case 2:
            dist=Cayley(permutationA, permutationB, m_size,m_invertedB,m_composition,m_maxPosInCycle, m_elemsToCycles, m_freeCycle);
            break;
        default:
            break;
    }
    
    return dist;
    
}
/*
 * Calculate distance between the permutation to the optimum.
 */
int API::CalculateDistanceToOptimum(int*permutation){
    return CalculateDistances(permutation, m_matrix_ConsensusPerms[0]);
}
