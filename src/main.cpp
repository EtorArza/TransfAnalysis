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


#define EXTERN
#include "Parameters.h"


using namespace NEAT;
using namespace std;



void usage()
{

    cerr << "usage: \n ./neat path_of_config_file" << endl;
    cerr << "for example, \n ./neat \"config_files/test.ini\"" << endl;
    cerr << endl;
    exit(1);
}




int main(int argc, char *argv[])
{   

    cout << "\n\n---------- BEGIN LICENCE DISCLAIMER----------\n";
    cout << "This code is based on accneat. Although some small changes \n";
    cout << "have been made, most of the NEAT algorithm remains unchanged.\n" << endl;
	cout <<	"Accneat is a fork of Stanley et al.'s implementation with some \n";
    cout << "improvements such as delete mutations and speed improvements, available at \n";
    cout << "https://github.com/sean-dougherty/accneat\n" << endl << endl;
    cout << "This code also uses a configuration file parser inih, available at https://github.com/jtilly/inih\n" << endl;
    cout << "The authors of the modified software distributed here are in NO way affiliated with accneat or INIH. Please, \n";
    cout << "understand that the use of this software requires reading and accepting the licences of both accneat and INIH.\n" << endl << endl;
    cout << "INIH is distributed with BSD licence, and accneat with APACHE LICENCE 2.0\n";
    cout << "The source code provided here (excluding accneat and INIH) was made by Etor Arza.\n";
    cout << "To keep the licence stuff as painless as possible, the software part writen by \n";
    cout << "Etor Arza is distributed with APACHE LICENCE 2.0 too.\n\n\n";
    cout << "This modified software contains some parts of the PerMallows package by Ekhiñe Irurozki available at https://cran.r-project.org/web/packages/PerMallows/index.html" << endl;
    cout << "----------END LICENCE DISCLAIMER----------\n\n";

    system("rm -r controllers_trained_with_cut60_tai80b");



    #ifndef NDEBUG
    cout << "WARNING: Debug mode. Assertions enabled." << endl;
    #endif

    if (argc < 2)
    {
        cout << "Error, no configuration file provided.\n";
        exit(1);
    }else if (argc > 2)
    {
        cout << "Error, too many arguments provided. Provide only path to configuration file.";
        exit(1);

    }

    return 0;
}
