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



void set_other_params(){
  SEED = 2;
  MAX_TIME = 0.5;
  POPSIZE = 20;
  TABU_LENGTH = 40;
  BEST_FOUND_FITNESS = -MAX_INTEGER;
  N_REPEATED_EVALS = _N_REPEATED_EVALS;
  REPEATED_EVALUATIONS[0] = 1;
  REPEATED_EVALUATIONS[1] = 3;
  REPEATED_EVALUATIONS[2] = 5;
  REPEATED_EVALUATIONS[3] = 9;
  REPEATED_EVALUATIONS[4] = 161;


}


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


bool parameters_set = false;
std::mutex mut;

// input parameters from bash
void set_parameters(int argc, char *argv[])
{
  if (parameters_set)
  {
    return;
  }
  
  mut.lock();
  if (parameters_set)
  {
    mut.unlock();
    return;
  }
  assert(N_OF_INPUT_PARAMS_TRAIN == argc || N_OF_INPUT_PARAMS_TEST == argc);
  // SEED = atoi(argv[i]);
  srand(SEED);
  PROBLEM_TYPE = std::string(argv[1]);
  INSTANCE_PATH = std::string(argv[2]);
  if (N_OF_INPUT_PARAMS_TEST == argc)
  {
      CONTROLLER_PATH = std::string(argv[3]);
  }

  set_other_params();
  mut.unlock();
  parameters_set = true;
}

// char const *params[3] = {"binary_name", "param_1", "param_2"}
void set_parameters(int argc, char const *argv[])
{

char* tmp[argc];

for (int i = 0; i < argc; i++)
{
  tmp[i] = (char*) argv[i];
}

set_parameters(argc, tmp);

}




std::string return_parameter_string(void)
{
  std::string result;
  result += "[";
  result += PROBLEM_TYPE;
  result += ", ";
  result += INSTANCE_PATH;
  result += ", ";
  result += std::to_string(SEED);
  result += ", ";
  result += std::to_string(MAX_TIME);
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