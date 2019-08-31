
#pragma once

#include <string>


#ifdef DEFINE_GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif



#define N_OF_INPUT_PARAMS 3


// CONSTANT PARAMETERS //
EXTERN int SEED;
EXTERN float MAX_TIME;
EXTERN int POPSIZE;
/////////////////////////

// BASH INPUT PARAMETERS //
// binary name
EXTERN std::string INSTANCE_PATH;
EXTERN std::string PROBLEM_TYPE;
///////////////////////////


// TECHNICAL PARAMETERS // 
#define DEBUG_MODE false
#define MAX_INTEGER 100000000
#define MIN_INTEGER -100000000
#define MAX_LONG_INTEGER 429496729500000
#define MIN_LONG_INTEGER -42949672950000
#define MAX(A,B) ( (A > B) ? A : B)
#define MIN(A,B) ( (A < B) ? A : B)
///////////////////////////



// argv[0] --> name of the binary executable
// argv[1] --> type of problem, i.e. LOP
// argv[2] --> path to the problem
void set_parameters(int argc, char *argv[]);
std::string return_parameter_string(void);
void print_parameters(void);






// NEAT PARAMETERS //
# define CUTOFF_0 0.25
//////////////////////
