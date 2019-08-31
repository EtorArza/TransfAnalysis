/*
 *  QAP.cpp
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 7/11/13.
 *  Copyright 2013 University of the Basque Country. All rights reserved.
 *
 */

#include "QAP.h"

/*
 *Class constructor.
 */
QAP::QAP()
{
	
}

/*
 * Class destructor.
 */
QAP::~QAP()
{
	for (int i=0;i<m_size;i++)
	{
		delete [] m_distance_matrix[i];
		delete [] m_flow_matrix[i];
	}
	delete [] m_flow_matrix;
	delete [] m_distance_matrix;

}



int QAP::Read(string filename)
{
	char line[5096]; // variable for input value
	ifstream indata;
	indata.open(filename.c_str(),ios::in);
	int num=0;
	while (!indata.eof())
	{
		//LEER LA LINEA DEL FICHERO
		indata.getline(line, 5096);
		stringstream ss;
		string sline;
		ss << line;
		ss >> sline;
        //cout<<"line: "<<line<<endl;
		if (num==0)
		{
			//OBTENER EL TAMAÑO DEL PROBLEMA
			m_size = atoi(sline.c_str());
			m_distance_matrix = new int*[m_size];
			m_flow_matrix = new int*[m_size];
			for (int i=0;i<m_size;i++)
			{
				m_distance_matrix[i]= new int[m_size];
				m_flow_matrix[i] = new int[m_size];
			}
		}
		else if (1<=num && num<=m_size)
		{
			//LOAD DISTANCE MATRIX
			char * pch;
			pch = strtok (line," ");
			int distance=atoi(pch);
			m_distance_matrix[num-1][0]=distance;
			for (int i=1;i < m_size; i++)
			{
				pch = strtok (NULL, " ,.");
				distance=atoi(pch);
				m_distance_matrix[num-1][i]=distance;
			}
		}
		else if (num>m_size && num<=(2*m_size))
		{
			//LOAD FLOW MATRIX
			char * pch;
			pch = strtok (line," ");
			int weight=atoi(pch);
			m_flow_matrix[num-m_size-1][0]=weight;
			for (int i=1;i < m_size; i++)
			{
				pch = strtok (NULL, " ,.");
				weight=atoi(pch);
				m_flow_matrix[num-m_size-1][i]=weight;
			}
		}
 
		else
		{
			break;
		}/*
        //LOAD DISTANCE MATRIX
        else{
            int distance;
            if (row>=m_size){
                //flow_matrix
                char * pch;
                pch = strtok (line," ");
                while (pch != NULL)
                {
                    distance=atoi(pch);
                    m_flow_matrix[row-m_size][col]=distance;
                    //cout<<"flow: "<<distance<<" row: "<<row<<" col: "<<col<<endl;
                    col++;
                    if (col==m_size){
                        col=0;
                        row++;
                    }
                    pch = strtok (NULL, " ,.");
                }
            }
            else{
                //distance_matrix
                char * pch;
                pch = strtok (line," ");
                while (pch != NULL)
                {
                    distance=atoi(pch);
                 //   cout<<"dist: "<<distance<<" row: "<<row<<" col: "<<col<<endl;
                    m_distance_matrix[row][col]=distance;
                    col++;
                    if (col==m_size){
                        col=0;
                        row++;
                    }
                    pch = strtok (NULL, " ,.");
                }
            }
        }*/
        
		num++;
	}
    //PrintMatrix(m_distance_matrix, m_size, m_size, "");
    //PrintMatrix(m_flow_matrix, m_size, m_size, "");
    //exit(1);
	initialize_variables_PBP(m_size);


	return (m_size);
}

/*
 * This function evaluates the individuals for the QAP problem.
 */
float QAP::Evaluate(int * genes)
{
	float fitness=0;
	int FactA, FactB;
	int distAB, flowAB, i ,j;
	for (i=0;i<m_size;i++)
	{
		for (j=0;j<m_size;j++)
		{
			FactA = genes[i];
			FactB = genes[j];
			
			//distAB= m_distance_matrix[i][j];
			//flowAB= m_flow_matrix[FactA][FactB];
            distAB= m_distance_matrix[FactA][FactB];
            flowAB= m_flow_matrix[i][j];
            
			fitness= fitness+(distAB*flowAB);			
		}
	}
	
	return -fitness;
}



/*
 * Returns the size of the problem.
 */
int QAP::GetProblemSize()
{
    return m_size;
}

