#include "cpunetwork.h"
#include "PERMU_params.h"

#pragma once

class PBP;
PBP *GetProblemInfo(std::string problemType, std::string filename);
double FitnessFunction_permu(NEAT::CpuNetwork *net, int n_evals, int seed, PERMU::params *parameters);
