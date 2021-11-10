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
	MultidimBenchmarkFF(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE);
	virtual ~MultidimBenchmarkFF() = 0;
    int GetProblemSize();
    double Fitness_Func_0_1(double* x_vec_0_1);
    void Evaluate(CIndividual* indiv);

    void load_rng(RandomNumberGenerator* rng);
    RandomNumberGenerator* rng;
    double ** R = nullptr;

    // get random rotation matrix.
    void getRandomRotationMatrix(int SEED);

  protected:

    // rotate x given rotation matrix R. x and res_x_rotated can point to the same address.
    void rotate_x_given_R(double *res_x_rotated, double *x);
    double* temp_vect_1;
    double get_x_lim_upper();
    double get_x_lim_lower();
    int dim;
    bool rng_deleted = false;
    bool rotate = false;

  private:
    virtual double FitnessFunc(double* x_vec) = 0;
    double x_upper_lim;
    double x_lower_lim;


};

MultidimBenchmarkFF* load_problem_with_default_lims(int problem_index, int dim,  int SEED, bool ROTATE);
MultidimBenchmarkFF* load_problem(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE);

class FRandomlyGenerated : public MultidimBenchmarkFF
{
public:

    FRandomlyGenerated(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE);
    ~FRandomlyGenerated();
    double FitnessFunc(double *x_vec);
};


class F1 : public MultidimBenchmarkFF
{
public:
    F1(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F1(){};
    double FitnessFunc(double *x_vec);
};

class F2 : public MultidimBenchmarkFF
{
public:
    F2(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F2(){};
    double FitnessFunc(double *x_vec);
};

class F3 : public MultidimBenchmarkFF
{
public:
    F3(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F3(){};
    double FitnessFunc(double *x_vec);
};

class F4 : public MultidimBenchmarkFF
{
public:
    F4(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F4(){};
    double FitnessFunc(double *x_vec);
};

class F5 : public MultidimBenchmarkFF
{
public:
    F5(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F5(){};
    double FitnessFunc(double *x_vec);
};

class F6 : public MultidimBenchmarkFF
{
public:
    F6(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F6(){};
    double FitnessFunc(double *x_vec);
};

class F7 : public MultidimBenchmarkFF
{
public:
    F7(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F7(){};
    double FitnessFunc(double *x_vec);
};

class F8 : public MultidimBenchmarkFF
{
public:
    F8(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F8(){};
    double FitnessFunc(double *x_vec);
};

class F9 : public MultidimBenchmarkFF
{
public:
    F9(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F9(){};
    double FitnessFunc(double *x_vec);
};

class F10 : public MultidimBenchmarkFF
{
public:
    F10(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F10(){};
    double FitnessFunc(double *x_vec);
};

class F11 : public MultidimBenchmarkFF
{
public:
    F11(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F11(){};
    double FitnessFunc(double *x_vec);
};

class F12 : public MultidimBenchmarkFF
{
public:
    F12(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE) : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE){};
    ~F12(){};
    double FitnessFunc(double *x_vec);
};
