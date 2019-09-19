
#include <string>
#include "PBP.h"
#include <mutex>
#ifndef EXTERN
#define EXTERN extern
#endif



#define N_OF_INPUT_PARAMS_TRAIN 3
#define N_OF_INPUT_PARAMS_TEST 5
#define _N_REPEATED_EVALS 5

class stopwatch;
// CONSTANT PARAMETERS //
EXTERN float MAX_TIME;
EXTERN float MIN_TIME;
EXTERN float CURRENT_TIME;
EXTERN int POPSIZE;
EXTERN int REPEATED_EVALUATIONS[_N_REPEATED_EVALS];
EXTERN int N_REPEATED_EVALS;
EXTERN int TABU_LENGTH;
EXTERN float FINAL_MAX_TIME;
/////////////////////////

// BASH INPUT PARAMETERS //
// binary name
EXTERN std::string INSTANCE_PATH;
EXTERN std::string PROBLEM_TYPE;
EXTERN std::string CONTROLLER_PATH;
///////////////////////////

// TRAIN PARAMS //
EXTERN float MAX_TRAIN_TIME;
EXTERN stopwatch global_timer;
///////////////////////////

// TECHNICAL PARAMETERS // 
#define MAX_LONG_INTEGER 429496729500000
#define MIN_LONG_INTEGER -42949672950000
#define MAX(A,B) ( (A > B) ? A : B)
#define MIN(A,B) ( (A < B) ? A : B)

// NEAT PARAMETERS //
# define CUTOFF_0 0.25
EXTERN float BEST_FOUND_FITNESS;
//////////////////////

///////////////////////////


// argv[0] --> name of the binary executable
// argv[1] --> type of problem, i.e. LOP
// argv[2] --> path to the problem
void set_parameters(int argc, char *argv[]);
void set_parameters(int argc, char const *argv[]);
std::string return_parameter_string(void);
void print_parameters(void);



PBP *GetProblemInfo(std::string problemType, std::string filename);



