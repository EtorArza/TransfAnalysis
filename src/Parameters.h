
#include <string>
#ifndef EXTERN
#define EXTERN extern
#endif

// FILE CONTAINING ALL GLOBAL VARIABLES and PARAMETERS

class stopwatch;


// PARAMS TO BE SPECIFIED IN CONFIG FILE//
EXTERN int N_OF_THREADS;
EXTERN double MAX_TRAIN_TIME;
EXTERN int POPSIZE_NEAT;
EXTERN stopwatch global_timer;

///////////////////////////

// Global variables (Not initialized by the user) // 
EXTERN double BEST_FITNESS_TRAIN;
EXTERN double N_TIMES_BEST_FITNESS_IMPROVED_TRAIN;
EXTERN double* F_VALUES_OBTAINED_BY_BEST_INDIV;
EXTERN std::string EXPERIMENT_FOLDER_NAME;



//////////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////

// NN consider value near 0 //
# define CUTOFF_0 0.25
//////////////////////

// MACROS // 
#define MAX(A,B) ( (A > B) ? A : B)
#define MIN(A,B) ( (A < B) ? A : B)
///////////