#define DEFINE_GLOBALS
#include "Parameters.h"
#include <string>
#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "Tools.h"
#include "PBP.h"
#include "LOP.h"
#include "TSP.h"
#include "QAP.h"
#include <float.h>





PBP *GetProblemInfo(std::string problemType, std::string filename)
{
    PBP *problem;
    // if (problemType == "pfsp")
    //     problem = new PFSP();
    // else if (problemType == "tsp")
    //     problem = new TSP();else
    if (problemType == "qap")
        {problem = new QAP();}
    else if (problemType == "lop")
        {problem = new LOP();}
    // else if (problemType == "api")
    //     problem = new API();
    else
    {
         cout << "Wrong problem type was specified." << endl;
         exit(1);
     }

    //Read the instance.
    problem->Read_with_mutex(filename);
    return problem;
}




std::string return_parameter_string(void)
{
  std::string result;
  result += "[";
  result += PROBLEM_TYPE;
  result += ", ";
  result += INSTANCE_PATH;
  result += ", ";
  result += std::to_string(MAX_TIME_PSO);
  result += ", ";
  result += std::to_string(POPSIZE);
  result += ", ";
  result += "']\n";
  return result;
}

void print_parameters(void)
{
  std::cout << return_parameter_string();
}