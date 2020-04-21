#define SAME_SIZE_EXPERIMENT

/*
  Copyright 2001 The University of Texas at Austin

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#include "std.h" // Must be included first. Precompiled header with standard library includes.
#include <unistd.h>
#include "Tools.h"
#include "FitnessFunction_permu.h"
#include "experiment.h"
#include "rng.h"
#include "evaluatorexperiment.h"
#include "INIReader.h"
#include "Parameters.h"

void usage()
{
    using namespace std;
    cerr << endl;
    cerr << "Usage: \n ./neat path_of_config_file" << endl;
    cerr << "for example, \n ./neat \"config_files/test.ini\"" << endl;
    cerr << "to delete experiment folder with the same name, use: \n";
    cerr << "\n ./neat -f \"config_files/test.ini\"" << endl;
    cerr << endl;
    exit(1);
}

int main(int argc, char *argv[])
{

 

    using namespace std;
    using namespace NEAT;

    cout << "\n---------- BEGIN LICENCE DISCLAIMER----------\n";
    cout << "This code is based on accneat. Although some small changes \n";
    cout << "have been made, most of the NEAT algorithm remains unchanged.\n"
         << endl;
    cout << "Accneat is a fork of Stanley et al.'s implementation with some \n";
    cout << "improvements such as delete mutations and speed improvements, available at \n";
    cout << "https://github.com/sean-dougherty/accneat (APACHE LICENCE 2.0)\n"
         << endl
         << endl;
    cout << "This software also uses some other software projects or parts of them." << endl;
    cout << "***\n";
    cout << "Configuration file parser inih, available at https://github.com/jtilly/inih (BSD licence)" << endl;
    cout << "Some parts of the PerMallows package by Ekhiñe Irurozki available at https://cran.r-project.org/web/packages/PerMallows/index.html (GLP licence)" << endl;
    cout << "The incomplete gamma function (asa032.cpp and asa032.hpp), by G Bhattacharjee, originally written by John Burkardt, (GNU LGPL license).\n";
    cout << "Wilcoxon singed test (WilcoxonTest.h, WilcoxonTest.h), by Scott Sauyet and Michael Hurley (MIT license) ";
    cout << "***\n";

    cout << "The original part of the source code provided here was made by Etor Arza.\n";
    cout << "To keep the licence stuff as painless as possible, the software part writen by \n";
    cout << "Etor Arza is distributed with APACHE LICENCE 2.0 too.\n\n";
    cout << "----------END LICENCE DISCLAIMER----------\n\n\n\n\n\n";

#ifndef NDEBUG
    cout << "WARNING: Debug mode. Assertions enabled." << endl;
#endif

    //system("rm -r controllers*");

    if (argc < 2)
    {
        cout << "Error, no configuration file provided.\n";
        exit(1);
    }
    else if (argc > 3)
    {
        cout << "Error, too many arguments provided."
             << "Provide only path to configuration file and optionally "
             << "use -f command to delete previous experiment.";
        exit(1);
    }

    std::string conf_file_path;
    neat_parameters* neat_params = new neat_parameters();
    INIReader reader;
    neat_params->DELETE_PREV_EXPERIMENT_FOLDER = false;


    if (std::string(argv[1]) == "-f" && argc == 2)
    {
        cout << "Error, no configuration file provided.\n";
        exit(1);
    }else if(argc == 2){
        conf_file_path = argv[1];
        reader = INIReader(argv[1]);
    }
    else if (std::string(argv[1]) == "-f" && argc == 3)
    {
        conf_file_path = argv[2];
        reader = INIReader(argv[2]);
        neat_params->DELETE_PREV_EXPERIMENT_FOLDER = true;
    }
    else if (std::string(argv[2]) == "-f" && argc == 3)
    {
        conf_file_path = argv[1];
        reader = INIReader(argv[1]);
        neat_params->DELETE_PREV_EXPERIMENT_FOLDER = true;
    }
    else
    {
        cout << "Error, more than one argument provided, and one of the arguments was not -f.";
        usage();
        exit(1);
    }

    int rng_seed = 2;

    if (reader.ParseError() != 0)
    {
        std::cout << "Can't load configuration file: " << argv[1] << "\n";
        exit(1);
    }

    std::string prob_name = reader.Get("Global", "PROBLEM_NAME", "UNKNOWN");
    if (!vector_contains_item(Experiment::get_names(), prob_name))
    {
        cout << "problem: " << prob_name << " not defined.\n";
        cout << "available experiments are: ";
        print_vector(Experiment::get_names());
        cout << "\nExiting..." << endl;
        exit(1);
    }
    else
    {
        cout << "Working on problem: " << prob_name << endl;
    }

    Experiment *exp = Experiment::get(prob_name.c_str());
    exp->neat_params = neat_params;
    rng_t rng{rng_seed};
    exp->run_given_conf_file(conf_file_path);
    delete neat_params;
    return 0;
}
