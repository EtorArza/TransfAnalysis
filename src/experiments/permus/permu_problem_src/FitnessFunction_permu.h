#include "cpunetwork.h"
#pragma once

class PBP;
PBP *GetProblemInfo(std::string problemType, std::string filename);
double FitnessFunction_permu(NEAT::CpuNetwork *net, int n_evals, int seed);
