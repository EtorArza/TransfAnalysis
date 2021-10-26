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
#include "generator.h"
#include "MultidimBenchmarkFF.h"



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
    // int n = 13;

    // int sigma_1[n] = {5,11,2,9,4,6,0,7,8,3,10,1,12};
    // int sigma_2[n] = {12,11,10,9,8,7,6,5,4,3,2,1,0};

    // cout << Kendall(sigma_1, sigma_2, n) / (double) (n*(n-1) / 2) << endl;
    // cout << l1_distance(sigma_1, sigma_2, n) / (double) (n*n / 2) << endl;
    // cout << n*n / 2 << endl;
    // exit(1);

    // // Rokkonen plot function
    // char config_path[] = "src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat";
    // g_seeded_initialize(config_path, 501, 2);
    // double x[2] = {stod(argv[1]),stod(argv[2])}; 
    // cout << g_calculate(x) << endl;
    // exit(0);

    // F7 rosembrock_test = F7(3, -2.048,2.048);
    // double x[3] = {1.0,1.0,1.2};
    // cout << rosembrock_test.FitnessFunc(x);
    // exit(1);

    // int n = 15;
    // int y = 5;

    // cout << p_sign_test(n,y) << endl;
    // exit(1);


    #ifdef HIPATIA
    cout << "HIPATIA is defined" << endl;
    #else
    cout << "HIPATIA is NOT defined" << endl;
    #endif




    using namespace std;
    using namespace NEAT;

    cout << "\n---------- BEGIN LICENCE DISCLAIMER----------\n";
    cout << "The neuroevolution algorithm is based in accneat. Although some small changes \n";
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
    cout << "Continuous optimization problem generator, by Jani Rönkkönen, available at http://ronkkonen.com/generator/ (MIT licence)" << endl;
    cout << "***\n";

    cout << "The original part of the source code provided here was made by Etor Arza.\n";
    cout << "To keep the licence stuff as painless as possible, the software part writen by \n";
    cout << "Etor Arza is distributed with APACHE LICENCE 2.0.\n\n";
    cout << "----------END LICENCE DISCLAIMER----------\n\n\n\n\n\n";

#ifndef NDEBUG
    cout << "WARNING: Debug mode. Assertions enabled." << endl;
#endif

    //system("rm -r controllers*");

    if (argc < 2)
    {
        cout << "Error, no configuration file provided.\n";
        printf("double uses %d bytes in this processor/compiler.\n", sizeof(double));           // some compilers print 8
        printf("long double uses %d bytes in this processor/compiler.\n", sizeof(long double)); // some compilers print 16
        exit(1);
        
        /* # check optimum of MCCORMICK function (plate-shaped)
        long double x_vec[2] = {-0.54719755140825202904776222712079913890192983672022819519043L, -1.54719755140825202910197233574507436060230247676372528076172L};
        int n = 10000000;
        long double delta = 0.0000000001;
        long double best_res = 1.9133;
        cout << std::setprecision(60) << std::flush;

        for (size_t i = 0; i < n; i++)
        {
            if (i % 10000 == 0)
                cout << (double)i / (double)n << endl;
            size_t j = i;

            long double res = 0;

            long double x1 = x_vec[0] + delta * (long double)((long double)i - (long double)n / 2) * 2 / (long double)n;
            long double x2 = x_vec[1] + delta * (long double)((long double)j - (long double)n / 2) * 2 / (long double)n;

            long double val1 = 0.866025403572777245327692779985895299432741012424230575561523 + sin(x1 + x2);
            long double val2 = -1.0L                                                          + pow(x1 - x2, 2);
            long double val3 = 2.04719755140825936980120136610139525146223604679107666015625 - 1.5L * x1 + 2.5L * x2 + 1.0L;

            res = val1 + val2 + val3;

            if (res < best_res)
            {
                best_res = res;
                cout << x1 << ", " << x2 << endl;
                cout << "val1 = " << val1 << endl;
                cout << "val2 = " << val2 << endl;
                cout << "val3 = " << val3 << endl;
                cout << "f(x) = " << -res << endl;
            }
        }
        */

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
    neat_params->load_global_params(conf_file_path);
    exp->neat_params = neat_params;
    global_rng.seed(neat_params->SEED);
    rng_t rng{neat_params->SEED};
    exp->run_given_conf_file(conf_file_path);
    delete neat_params;
    return 0;
}
