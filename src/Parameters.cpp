#include "std.h"
#include "Parameters.h"
#include "Tools.h"
#include "INIReader.h"
#include <cfloat>
#include "util.h"
#include "neat.h"
#include <stdio.h>

void load_global_params(std::string conf_file_path)
{
    INIReader reader(conf_file_path);
    N_OF_THREADS = reader.GetInteger("NEAT", "THREADS", -1);
    N_OF_THREADS = MAX(N_OF_THREADS, reader.GetInteger("TestSettings", "THREADS", -1));
    MAX_TRAIN_TIME = reader.GetInteger("NEAT", "MAX_TRAIN_TIME", -1);
    POPSIZE_NEAT = reader.GetInteger("NEAT", "POPSIZE", -1);
    global_timer.tic();
    NEAT::env->pop_size = POPSIZE_NEAT;
}

void delete_prev_exp_folder()
{
    if (exists(EXPERIMENT_FOLDER_NAME))
    {
        if (DELETE_PREV_EXPERIMENT_FOLDER)
        {
            system(std::string("rm -r " + EXPERIMENT_FOLDER_NAME).c_str());
        }
        else
        {
            std::cout << "Already exists: " + EXPERIMENT_FOLDER_NAME + ".\nMove your experiment directories or use -f to delete them automatically. If -f is used, all previous experiments will be deleted.";
        }
    }
}