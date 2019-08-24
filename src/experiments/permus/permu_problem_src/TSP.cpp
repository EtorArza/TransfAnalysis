/*
 *  TSP.cpp
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 7/11/13.
 *  Copyright 2013 University of the Basque Country. All rights reserved.
 *
 */

#include "TSP.h"


/*
 *Class constructor.
 */
TSP::TSP()
{
	
}

/*
 * Class destructor.
 */
TSP::~TSP()
{
	for (int i=0;i<m_size;i++)
        delete [] m_distance_matrix[i];
    delete [] m_distance_matrix;

}


double CalculateGEODistance(double latitudeX, double latitudeY, double longitudeX, double longitudeY)
{
	double PI = 3.141592;
	double RRR = 6378.388;
	
	double deg = (double)((int)latitudeX);
	double min = latitudeX - deg;
	double latitudeI = PI * (deg + 5.0 * min / 3.0 ) / 180.0;
	
	deg = (double)((int)latitudeY);
	min = latitudeY - deg;
	double longitudeI = PI * (deg + 5.0 * min / 3.0 ) / 180.0;
	
	deg = (double)((int)longitudeX);
	min = longitudeX - deg;
	double latitudeJ = PI * (deg + 5.0 * min / 3.0 ) / 180.0;
	
	deg = (double)((int)longitudeY);
	min = longitudeY - deg;
	double longitudeJ = PI * (deg + 5.0 * min / 3.0 ) / 180.0;
	
	double q1 = cos(longitudeI - longitudeJ);
	double q2 = cos(latitudeI - latitudeJ);
	double q3 = cos(latitudeI + latitudeJ);
	
	return (int)(RRR * acos(0.5 * ((1.0+q1) * q2 - (1.0 - q1) * q3)) + 1.0);
}

/*
 * Read TSP instance file that belongs to the TSPLIB library.
 */
int TSP::Read2(string filename)
{
    //declaration and initialization of variables.
	bool readm_distance_matrix=false;
	bool coordinatesData=false;
	string DISTANCE_TYPE;
	double **coordinates;
	char line[2048]; // variable for input value
	string data="";
	ifstream indata;
	indata.open(filename.c_str(),ios::in);
	
	while (!indata.eof())
	{
		//LEER LA LINEA DEL FICHERO
		indata.getline(line, 2048);
		stringstream ss;
		string sline;
		ss << line;
		ss >> sline;
		if (sline=="EOF")
		{
			break;
		}
		if (readm_distance_matrix && coordinatesData==false)
		{
			//cout << "reading distance m_distance_matrix "<<line<< endl;
			if (data=="")
				data = line;
			else
				data = data+' '+line;
		}
		else if (readm_distance_matrix && coordinatesData==true)
		{
			//FILL DISTANCE m_distance_matrix
			char * coordPieces;
			coordPieces = strtok (line," ");
			if (coordPieces==" ")
			{
				coordPieces = strtok (NULL," ");
			}
			int coordNum = atoi(coordPieces);
			coordPieces = strtok (NULL, " ");
			double latitud = atof(coordPieces);
			coordPieces = strtok (NULL, " ");
			double longitud = atof(coordPieces);
			double *coordinate= new double[2];
			coordinate[0]=latitud;
			coordinate[1]=longitud;
			
			coordinates[coordNum-1]= coordinate;
			//cout<<"coordNum "<<coordNum-1<<" latit: "<<latitud<<" long: "<<longitud<<endl;
		}
		
		if(strContains(sline,"DIMENSION"))
		{
			char * pch;
			pch = strtok (line," ");
			pch = strtok (NULL, " ");
			if (strcmp(pch,":")==0)
			{
				pch = strtok (NULL, " ");
			}
			m_size = atoi(pch);
		}
		else if (strContains(sline,"EDGE_WEIGHT_TYPE"))
		{
			char * pch;
			pch = strtok (line," ");
			pch = strtok (NULL, " ");
			if (strcmp(pch,":")==0)
			{
				pch = strtok (NULL, " ");
			}
			stringstream s;
			string type;
			s << pch;
			s >> type;
			DISTANCE_TYPE = type;
		}
        else if (sline =="EDGE_WEIGHT_SECTION")
		{
			readm_distance_matrix=true;
			coordinatesData=false;
		}
		else if (sline=="NODE_COORD_SECTION"){
			readm_distance_matrix=true;
			coordinatesData=true;
			coordinates= new double*[m_size];
		}
		
	}
	indata.close();
	
	//BUILD DISTANCE m_distance_matrix
	m_distance_matrix = new double*[m_size];
	for (int i=0;i<m_size;i++)
	{
		m_distance_matrix[i]= new double[m_size];
	}
	
	//FILL DISTANCE m_distance_matrix
	if (coordinatesData==true)
	{
		//CALCULATE EUCLIDEAN DISTANCES
		for (int i=0;i<m_size;i++)
		{
			//get coordinate A
			double *coordA=coordinates[i];
			double coordAx = coordA[0];
			double coordAy = coordA[1];
			for (int j=i;j<m_size;j++)
			{
				//get coordinate B.
				double *coordB=coordinates[j];
				double coordBx=coordB[0];
				double coordBy=coordB[1];
				double euclidean;
				if (DISTANCE_TYPE=="GEO")
				{
					//calculate geographic distance between A and B.
					euclidean=CalculateGEODistance(coordAx, coordAy, coordBx, coordBy);
				}
				else
				{
					//calculate euclidean distance between A and B.
					double absolute= fabs(pow((coordAx-coordBx),2) + pow((coordAy-coordBy),2));
					euclidean= sqrt(absolute);
                }
				m_distance_matrix[i][j]=  euclidean;
				m_distance_matrix[j][i]= euclidean;//<-symmetric m_distance_matrix
			}
		}
	}
	else
	{
		//FILL DISTANCE m_distance_matrix
		istringstream iss(data);
		int i=0;
		int j=0;
		do
		{
			string sub;
		    iss >> sub;
		    if (sub!=""){
				//save distance in distances m_distance_matrix. Save negative distance in order to minimize fitness instead of
				//maximize.
		    	m_distance_matrix[i][j]= atoi(sub.c_str());
		    	m_distance_matrix[j][i]= atoi(sub.c_str());//<-symmetric m_distance_matrix
		    	if (sub=="0")
		    	{
		    		i++;
		    		j=0;
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
	}
 
 	initialize_variables_PBP(m_size);


    return (m_size);
}



/*
 * Read TSP instance file.
 */
int TSP::Read(string filename)
{
	char line[2048]; // variable for input value
	string data="";
	ifstream indata;
	indata.open(filename.c_str(),ios::in);
	int num=0;
	while (!indata.eof())
	{
		indata.getline(line, 2048);
		stringstream ss;
		string sline;
		ss << line;
		ss >> sline;
		if (sline=="")
		{
			break;
		}
		if (num==0)
		{
			m_size = atoi(line);
		}
		else
		{
			if (data=="")
				data = line;
			else
				data = data+' '+line;
		}
		num++;
	}
	indata.close();
    
	//BUILD MATRIX
	m_distance_matrix = new double*[m_size];
	for (int i=0;i<m_size;i++)
	{
		m_distance_matrix[i]= new double[m_size];
	}
    
	istringstream iss(data);
	int i=0;
	int j=0;
	do
	{
		string sub;
	    iss >> sub;
	    if (sub!=""){
			//save distance in distances matrix.
	    	m_distance_matrix[i][j]= atoi(sub.c_str());
	    	if (j==(m_size-1))
	    	{
	    		i++;
	    		j=0;
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
	return (m_size);
}

 /*
 * This function evaluates the fitness of the solution for the TSP problem.
 */
double TSP::Evaluate(int * genes)
{
	double distanceSum=0;
	double distAB=0;
	int IDCityA, IDCityB;
	for (int i=0;i<m_size;i++)
	{
		IDCityA = genes[i];
		IDCityB = genes[0];
		if (i+1<m_size)
		{
			IDCityB = genes[i+1];
		}
		
		distAB = m_distance_matrix[IDCityA][IDCityB];
		distanceSum=distanceSum+distAB;
		
		//security condition
		if (IDCityA==m_size || IDCityB==m_size){
			distanceSum=0;
			break;
		}
	}
	return -distanceSum;
}



/*
 * Returns the size of the problem.
 */
int TSP::GetProblemSize()
{
    return m_size;
}

