#include "Tools.h"
#include <math.h>
#include <assert.h>
#include "MultidimBenchmarkFF.h"

MultidimBenchmarkFF::MultidimBenchmarkFF(int dim)
{
    this->dim = dim;
    rng = new RandomNumberGenerator();
    temp_vect_1 = new double[this->dim];
}


MultidimBenchmarkFF::~MultidimBenchmarkFF()
{
    if (!this->rng_deleted)
    {
        delete this->rng;
        this->rng_deleted = true;
    }
    delete[] temp_vect_1;
}


void MultidimBenchmarkFF::load_rng(RandomNumberGenerator *rng)
{
    if (!this->rng_deleted)
    {
        delete this->rng;
        this->rng_deleted = true;
    }
    this->rng = rng;
}

int MultidimBenchmarkFF::GetProblemSize() { return this->dim; }


double MultidimBenchmarkFF::Fitness_Func_0_1(double* x_vec_0_1){
    for (int i = 0; i < this->dim; i++)
    {   
        assert(x_vec_0_1[i] > -0.0000001);
        assert(x_vec_0_1[i] < 1.0000001);
        temp_vect_1[i] = this->get_x_lim_lower() + x_vec_0_1[i] * (this->get_x_lim_upper() - this->get_x_lim_lower());
    }
    return - this->FitnessFunc(temp_vect_1); // we need the - sign because we are considering minimmization problems.
}


void MultidimBenchmarkFF::Evaluate(CIndividual *indiv)
{
	double fitness = 0;
	fitness = Fitness_Func_0_1(indiv->genome);
	indiv->f_value = fitness;
    indiv->bk_was_improved = false;
    if (fitness >indiv->f_best)
    {
        indiv->bk_was_improved = true;
        indiv->f_best = fitness;
        copy_array(indiv->genome_best, indiv->genome, indiv->n);
    }
}



// Sphere
double F1::get_x_lim_upper(){return 5.12;};
double F1::get_x_lim_lower(){return -5.12;};
double F1::FitnessFunc(double* x_vec){
    double res = 0;
    for (int i = 0; i < dim; i++)
    {
        res += x_vec[i] * x_vec[i];
    }
    return res;
}


// Dixon & price
double F2::get_x_lim_upper(){return 10.0;};
double F2::get_x_lim_lower(){return -10.0;};
double F2::FitnessFunc(double* x_vec){
    double res = (x_vec[0] - 1.0) * (x_vec[0] - 1.0);
    for (int i = 2; i <= dim; i++)
    {
        res += (double) i * (2.0 * x_vec[i-1] * x_vec[i-1] - x_vec[i-2]) * (2.0 * x_vec[i-1] * x_vec[i-1] - x_vec[i-2]);
    }
    return res;
}



// Zakharov
double F3::get_x_lim_upper(){return 10.0;};
double F3::get_x_lim_lower(){return -5.0;};
double F3::FitnessFunc(double* x_vec){
    double val_1 = 0.0;
    double val_2 = 0.0;
    
    for (int i = 0; i < dim; i++)
    {
        val_1 += x_vec[i] * x_vec[i];
    }
    
    for (int i = 1; i <= dim; i++)
    {
        val_2 += (0.5 * i * x_vec[i-1]); 
    }

    return val_1 + (val_2 * val_2) + (val_2 * val_2 * val_2 * val_2);
}


// Rastrigin
double F4::get_x_lim_upper(){return 5.12;};
double F4::get_x_lim_lower(){return -5.12;};
double F4::FitnessFunc(double* x_vec){
    double res = (double) 10 * dim;

    for (int i = 0; i < dim; i++)
    {
        res += x_vec[i] * x_vec[i] - 10.0 * cos(2.0 * M_PI * x_vec[i]);
    }

    return res;
}


// Levy
double F5::get_x_lim_upper(){return 30.0;};
double F5::get_x_lim_lower(){return -15.0;};
double F5::FitnessFunc(double* x_vec){
    double w_1 = 1.0 + (x_vec[0] - 1.0)/4.0;
    double res = sin(M_PI * w_1) * sin(M_PI * w_1);

    for (int i = 0; i < dim - 1; i++)
    {
        double w_i = 1.0 + (x_vec[i] - 1.0)/4.0;
        res += (w_i - 1) *(w_i - 1) * (1.0 + 10.0*sin(M_PI * w_i + 1.0)*sin(M_PI * w_i + 1.0));
    }

    double w_d = 1.0 + (x_vec[dim - 1] - 1.0)/4.0;

    res += (w_d - 1.0) * (w_d - 1.0) * (1.0 + pow(sin(2.0 * M_PI * w_d), 2.0));

    return res;
}



// Griewank
double F6::get_x_lim_upper(){return 600.0;};
double F6::get_x_lim_lower(){return -600.0;};
double F6::FitnessFunc(double* x_vec){
    double val_1 = 0.0;
    for (int i = 0; i < dim; i++)
    {
        val_1 += x_vec[i] * x_vec[i] / 4000.0;
    }

    double val_2 = 1.0;

    for (int i = 1; i <= dim; i++)
    {
        val_2 *= cos(x_vec[i-1] / sqrt((double) i));
    }

    double res = 1.0 + val_1 - val_2;

    return res;
}


// Rosenbrock
double F7::get_x_lim_upper(){return 10.0;};
double F7::get_x_lim_lower(){return -5.0;};
double F7::FitnessFunc(double* x_vec){
    double res = 0.0;

    for (int i = 0; i < dim-1; i++)
    {
        res += 100.0 * 
        (x_vec[i]*x_vec[i] - x_vec[i+1]) * 
        (x_vec[i]*x_vec[i] - x_vec[i+1]) + 
        (x_vec[i] - 1.0) * 
        (x_vec[i] - 1.0);
    }



    return res;
}


// Ackley
double F8::get_x_lim_upper(){return 30.0;};
double F8::get_x_lim_lower(){return -15.0;};
double F8::FitnessFunc(double* x_vec){
    double sum_1 = 0.0;
    double sum_2 = 0.0;

    for (int i = 0; i < dim; i++)
    {
        sum_1 += x_vec[i] * x_vec[i];
    }

    sum_1 = -20 * exp(-0.2 * sqrt(sum_1 / (double) dim));

    for (int i = 0; i < dim; i++)
    {
        sum_2 += cos(2.0 * M_PI * x_vec[i]);
    }

    sum_2 = -exp(sum_2 / (double) dim);


    return 20.0 + exp(1) + sum_1 + sum_2;
}

