#define DEFINE_GLOBALS
#include "Parameters.h"
#include <string>
#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "Tools.h"




void set_other_params(){
  SEED = 2;
  MAX_TIME = 1;
  POPSIZE = 20;
}




// input parameters from bash
void set_parameters(int argc, char *argv[])
{
  assert(N_OF_INPUT_PARAMS == argc);
  // SEED = atoi(argv[i]);
  srand(SEED);
  PROBLEM_TYPE = std::string(argv[1]);
  INSTANCE_PATH = std::string(argv[2]);
  set_other_params();


}

// char const *params[3] = {"binary_name", "param_1", "param_2"}
void set_parameters(int argc, char const *argv[])
{
  assert(N_OF_INPUT_PARAMS == argc);
  // SEED = atoi(argv[i]);
  srand(SEED);
  PROBLEM_TYPE = std::string(argv[1]);
  INSTANCE_PATH = std::string(argv[2]);
  set_other_params();
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