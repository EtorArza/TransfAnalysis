
#include <string>
#ifndef EXTERN
#define EXTERN extern
#endif




class stopwatch;
// CONSTANT PARAMETERS //
EXTERN double MAX_TIME_PSO;
EXTERN int POPSIZE;
EXTERN int N_EVALS;
EXTERN int N_REEVALS;
EXTERN int TABU_LENGTH;
/////////////////////////

// BASH INPUT PARAMETERS //
// binary name
EXTERN std::string INSTANCE_PATH;
EXTERN std::string PROBLEM_TYPE;
EXTERN std::string CONTROLLER_PATH;
EXTERN int N_OF_THREADS;
///////////////////////////

// TRAIN PARAMS //
EXTERN double MAX_TRAIN_TIME;
EXTERN stopwatch global_timer;
///////////////////////////

// TECHNICAL PARAMETERS // 
#define MAX_LONG_INTEGER 429496729500000
#define MIN_LONG_INTEGER -42949672950000
#define MAX(A,B) ( (A > B) ? A : B)
#define MIN(A,B) ( (A < B) ? A : B)

// NEAT PARAMETERS //
# define CUTOFF_0 0.25
//////////////////////

