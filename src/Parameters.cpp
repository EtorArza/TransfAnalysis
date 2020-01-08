#include "Parameters.h"
#include "Tools.h"
#include "INIReader.h"
#include <cfloat>
#include "neat.h"


void load_global_params(std::string conf_file_path){
    INIReader reader(conf_file_path);
    N_OF_THREADS = reader.GetInteger("NEAT", "THREADS", -1);
    MAX_TRAIN_TIME = reader.GetInteger("NEAT", "MAX_TRAIN_TIME", -1);
    POPSIZE_NEAT = reader.GetInteger("NEAT", "POPSIZE", -1);
    global_timer.tic();
    NEAT::env->pop_size = POPSIZE_NEAT;
}