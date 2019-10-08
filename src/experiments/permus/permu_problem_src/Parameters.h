
#include <string>
#ifndef EXTERN
#define EXTERN extern
#endif


// FILE CONTAINING ALL GLOBAL VARIABLES and PARAMETERS

class stopwatch;

// PARAMETERS TO BE SPECIFIED IN CONFIG FILE//
EXTERN double MAX_TIME_PSO;
EXTERN int POPSIZE;
EXTERN int N_EVALS;
EXTERN int N_REEVALS_TOP_5_PERCENT;
EXTERN int N_EVALS_TO_UPDATE_BK;

EXTERN int TABU_LENGTH;
EXTERN std::string INSTANCE_PATH;
EXTERN std::string PROBLEM_TYPE;
EXTERN int N_OF_THREADS;
EXTERN bool START_WITHOUT_HIDDEN;
///////////////////////////

// TRAIN PARAMS TO BE SPECIFIED IN CONFIG FILE//
EXTERN double MAX_TRAIN_TIME;
EXTERN stopwatch global_timer;
///////////////////////////

// TEST PARAMS TO BE SPECIFIED IN CONFIG FILE//
EXTERN std::string CONTROLLER_PATH;
EXTERN int N_REPS;
/////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////



// Global variables (Not initialized by the user) // 
EXTERN double BEST_FITNESS_TRAIN;
EXTERN double N_TIMES_BEST_FITNESS_IMPROVED_TRAIN;
///////////////////////////////////////////////////////

// NN consider value near 0 //
# define CUTOFF_0 0.25
//////////////////////

// MACROS // 
#define MAX(A,B) ( (A > B) ? A : B)
#define MIN(A,B) ( (A < B) ? A : B)
///////////