
#include <string>
#ifndef EXTERN
#define EXTERN extern
#endif


void load_global_params(std::string conf_file_path);
void delete_prev_exp_folder();


// PARAMS TO BE SPECIFIED IN CONFIG FILE//
EXTERN int N_OF_THREADS;
EXTERN double MAX_TRAIN_TIME;
EXTERN int POPSIZE_NEAT;
EXTERN class stopwatch global_timer;



// Global variables // 
EXTERN double BEST_FITNESS_TRAIN;
EXTERN double N_TIMES_BEST_FITNESS_IMPROVED_TRAIN;
EXTERN std::string EXPERIMENT_FOLDER_NAME;
EXTERN bool DELETE_PREV_EXPERIMENT_FOLDER;






#define CUTOFF_0 0.25 // NN consider value near 0

#define SMALLEST_POSITIVE_DOUBLE 0.0000001
#define NETWORK_SIGNATURE_SIZE 10

// MACROS // 
#define MAX(A,B) ( (A > B) ? A : B)
#define MIN(A,B) ( (A < B) ? A : B)
///////////