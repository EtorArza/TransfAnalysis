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
	MultidimBenchmarkFF(int dim, double x_lower_lim, double x_upper_lim);
	virtual ~MultidimBenchmarkFF() = 0;
    int GetProblemSize();
    double Fitness_Func_0_1(double* x_vec_0_1);
    void Evaluate(CIndividual* indiv);
    virtual double FitnessFunc(double* x_vec) = 0;
    void load_rng(RandomNumberGenerator* rng);
    RandomNumberGenerator* rng;

  protected:

    double* temp_vect_1;
    double get_x_lim_upper();
    double get_x_lim_lower();
    int dim;
    bool rng_deleted = false;

  private:
    double x_upper_lim;
    double x_lower_lim;


};

class F1 : public MultidimBenchmarkFF
{
public:
    F1(int dim, double x_lower_lim, double x_upper_lim) : MultidimBenchmarkFF(dim, x_lower_lim, x_upper_lim){};
    ~F1(){};
    double FitnessFunc(double *x_vec);
};

class F2 : public MultidimBenchmarkFF
{
public:
    F2(int dim, double x_lower_lim, double x_upper_lim) : MultidimBenchmarkFF(dim, x_lower_lim, x_upper_lim){};
    ~F2(){};
    double FitnessFunc(double *x_vec);
};

class F3 : public MultidimBenchmarkFF
{
public:
    F3(int dim, double x_lower_lim, double x_upper_lim) : MultidimBenchmarkFF(dim, x_lower_lim, x_upper_lim){};
    ~F3(){};
    double FitnessFunc(double *x_vec);
};

class F4 : public MultidimBenchmarkFF
{
public:
    F4(int dim, double x_lower_lim, double x_upper_lim) : MultidimBenchmarkFF(dim, x_lower_lim, x_upper_lim){};
    ~F4(){};
    double FitnessFunc(double *x_vec);
};

class F5 : public MultidimBenchmarkFF
{
public:
    F5(int dim, double x_lower_lim, double x_upper_lim) : MultidimBenchmarkFF(dim, x_lower_lim, x_upper_lim){};
    ~F5(){};
    double FitnessFunc(double *x_vec);
};

class F6 : public MultidimBenchmarkFF
{
public:
    F6(int dim, double x_lower_lim, double x_upper_lim) : MultidimBenchmarkFF(dim, x_lower_lim, x_upper_lim){};
    ~F6(){};
    double FitnessFunc(double *x_vec);
};

class F7 : public MultidimBenchmarkFF
{
public:
    F7(int dim, double x_lower_lim, double x_upper_lim) : MultidimBenchmarkFF(dim, x_lower_lim, x_upper_lim){};
    ~F7(){};
    double FitnessFunc(double *x_vec);
};

class F8 : public MultidimBenchmarkFF
{
public:
    F8(int dim, double x_lower_lim, double x_upper_lim) : MultidimBenchmarkFF(dim, x_lower_lim, x_upper_lim){};
    ~F8(){};
    double FitnessFunc(double *x_vec);
};

class F9 : public MultidimBenchmarkFF
{
public:
    F9(int dim, double x_lower_lim, double x_upper_lim) : MultidimBenchmarkFF(dim, x_lower_lim, x_upper_lim){};
    ~F9(){};
    double FitnessFunc(double *x_vec);
};

class F10 : public MultidimBenchmarkFF
{
public:

    F10(int dim, double x_lower_lim, double x_upper_lim, int seed);
    ~F10();
    double FitnessFunc(double *x_vec);
};


