#pragma once

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include "Individual.h"
#include "Tools.h"

class CIndividual;


class MultidimBenchmarkFF
{


  public:
	MultidimBenchmarkFF(int dim);
	virtual ~MultidimBenchmarkFF() = 0;
    int GetProblemSize();
    double Fitness_Func_0_1(double* x_vec_0_1);
    void Evaluate(CIndividual* indiv);
    virtual double FitnessFunc(double* x_vec) = 0;
    void load_rng(RandomNumberGenerator* rng);
    RandomNumberGenerator* rng;

  protected:

    double* temp_vect_1;
    virtual double get_x_lim_upper() = 0;
    virtual double get_x_lim_lower() = 0;
    int dim;
    bool rng_deleted = false;

};

class F1 : public MultidimBenchmarkFF
{
public:
    F1(int dim) : MultidimBenchmarkFF(dim){};
    ~F1(){};
    double get_x_lim_upper();
    double get_x_lim_lower();
    double FitnessFunc(double *x_vec);
};

class F2 : public MultidimBenchmarkFF
{
public:

	F2(int dim):MultidimBenchmarkFF(dim){};
    ~F2(){};
    double get_x_lim_upper();
    double get_x_lim_lower();
    double FitnessFunc(double* x_vec);
};


class F3 : public MultidimBenchmarkFF
{
public:

	F3(int dim):MultidimBenchmarkFF(dim){};
    ~F3(){};
    double get_x_lim_upper();
    double get_x_lim_lower();
    double FitnessFunc(double* x_vec);
};


class F4 : public MultidimBenchmarkFF
{
public:

	F4(int dim):MultidimBenchmarkFF(dim){};
    ~F4(){};
    double get_x_lim_upper();
    double get_x_lim_lower();
    double FitnessFunc(double* x_vec);
};


class F5 : public MultidimBenchmarkFF
{
public:

	F5(int dim):MultidimBenchmarkFF(dim){};
    ~F5(){};
    double get_x_lim_upper();
    double get_x_lim_lower();
    double FitnessFunc(double* x_vec);
};


class F6 : public MultidimBenchmarkFF
{
public:

	F6(int dim):MultidimBenchmarkFF(dim){};
    ~F6(){};
    double get_x_lim_upper();
    double get_x_lim_lower();
    double FitnessFunc(double* x_vec);
};


class F7 : public MultidimBenchmarkFF
{
public:

	F7(int dim):MultidimBenchmarkFF(dim){};
    ~F7(){};
    double get_x_lim_upper();
    double get_x_lim_lower();
    double FitnessFunc(double* x_vec);
};

class F8 : public MultidimBenchmarkFF
{
public:

	F8(int dim):MultidimBenchmarkFF(dim){};
    ~F8(){};
    double get_x_lim_upper();
    double get_x_lim_lower();
    double FitnessFunc(double* x_vec);
};

