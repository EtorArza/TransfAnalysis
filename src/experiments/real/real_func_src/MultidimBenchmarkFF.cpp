#include "Tools.h"
#include <math.h>
#include <assert.h>
#include "MultidimBenchmarkFF.h"
#include "generator.h"
#include <omp.h>
#include <unistd.h>
MultidimBenchmarkFF::MultidimBenchmarkFF(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE)
{
    this->dim = dim;
    rng = new RandomNumberGenerator();
    temp_vect_1 = new double[this->dim];
    this->x_lower_lim = x_lower_lim;
    this->x_upper_lim = x_upper_lim;

    if (ROTATE)
    {
        this->getRandomRotationMatrix(SEED);
        this->rotate = true;
    }
    else
    {
        this->rotate = false;
    }
    if (x_lower_lim >= x_upper_lim)
    {
        cout << "\n Error in MultidimBenchmarkFF, x_lims = (" << x_lower_lim << ", " << x_upper_lim << ") not correct.\n" <<endl;
        exit(1);
    }




}


MultidimBenchmarkFF::~MultidimBenchmarkFF()
{
    if (R != nullptr)
    {
        delete_matrix(R, this->dim);
        R = nullptr;
    }
    
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

    double local_tmp_vec[this->dim]; 

    if (rotate)
    {
        rotate_x_given_R(x_vec_0_1, local_tmp_vec);
    }else
    {
        copy_array(local_tmp_vec, x_vec_0_1, this->dim);
    }
    



    for (int i = 0; i < this->dim; i++)
    {   
        assert(x_vec_0_1[i] > -0.0000001);
        assert(x_vec_0_1[i] < 1.0000001);
        local_tmp_vec[i] = this->get_x_lim_lower() + x_vec_0_1[i] * (this->get_x_lim_upper() - this->get_x_lim_lower());
    }







    

    return - this->FitnessFunc(local_tmp_vec); // we need the - sign because we are considering minimmization problems.
}



double MultidimBenchmarkFF::get_x_lim_lower()
{
    return this->x_lower_lim;
}


double MultidimBenchmarkFF::get_x_lim_upper()
{
    return this->x_upper_lim;
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
double F1::FitnessFunc(double* x_vec){
    double res = 0;
    for (int i = 0; i < dim; i++)
    {
        res += x_vec[i] * x_vec[i];
    }
    return res;
}


// Dixon & price
double F2::FitnessFunc(double* x_vec){
    double res = (x_vec[0] - 1.0) * (x_vec[0] - 1.0);
    for (int i = 2; i <= dim; i++)
    {
        res += (double) i * (2.0 * x_vec[i-1] * x_vec[i-1] - x_vec[i-2]) * (2.0 * x_vec[i-1] * x_vec[i-1] - x_vec[i-2]);
    }
    return res;
}



// Zakharov
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
double F4::FitnessFunc(double* x_vec){
    double res = (double) 10 * dim;

    for (int i = 0; i < dim; i++)
    {
        res += x_vec[i] * x_vec[i] - 10.0 * cos(2.0 * M_PI * x_vec[i]);
    }

    return res;
}


// Levy
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

// Quadratic
double F9::FitnessFunc(double* x_vec){
    double res = 0;
    double tmp;
    for (int i = 0; i < dim; i++)
    {
        tmp = 0;
        for (int j = 0; j < i+1; j++)
        {
            tmp += x_vec[j];
        }
        res += tmp * tmp;
    }
    return res;
}


static int current_jobs_with_this_seed = 0;
F11::F11(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE)  : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE)
{
    char config_path[] = "src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat";


    bool repeat = true;
    while (repeat)
    {
        #pragma omp critical(adshfafoiadfoak)
        {
            //cout << "(" << seed << "," << dim;
            static int current_dim = -1;
            static int current_seed = -1;
            if (current_jobs_with_this_seed == 0)
            {
                g_seeded_initialize(config_path, SEED, dim);
                current_seed = SEED;
                current_dim = dim;
                current_jobs_with_this_seed++;
                repeat = false;
            }
            else if(current_seed == SEED && current_dim == dim)
            {
                current_jobs_with_this_seed++;
                repeat = false;
            }
            else
            {
                //cout << "waiting";
                repeat = true;
            }
            //cout << endl;
        }
        if (repeat == true)
        {
            usleep((useconds_t) 1000 );
        }
        
    }

};


F11::~F11()
{
    #pragma omp critical(adshfafoiadfoak)
    {
        //cout << current_seed << "," << dim << ")" << endl;
        current_jobs_with_this_seed--;
    }
};


double F11::FitnessFunc(double* x_vec)
{
    return g_calculate(x_vec);
}



// Rosembrock EZ
double F10::FitnessFunc(double* x_vec){
    
    double res = 0;

    for (int i = 1; i <= dim/2; i++)
    {
        res += (100.0*pow(x_vec[2*i - 1]- x_vec[2*i -2]* x_vec[2*i -2],2) + pow(1.0 - x_vec[2*i -2],2)  );
    }
    return res;

}


void MultidimBenchmarkFF::getRandomRotationMatrix(int SEED)
{

    if (this->R==nullptr)
    {
        zero_initialize_matrix(R, this->dim, this->dim);
    }


  double scalar=0, line[this->dim], norm=0;
  int i,j,k;
  RandomNumberGenerator local_rng = RandomNumberGenerator();
  local_rng.seed(SEED);
  
  for (i=0;i<this->dim;i++){		
    /*random values for matrix R:s i:th row, first step*/
    for (j=0;j<this->dim;j++)
      R[i][j]=local_rng.random_0_1_double();		 
    
    /*second step*/
    /*if j<=i, the line must be 0*/
    for(k=0;k<this->dim;k++)		
      line[k]=0.0;
    
    for(j=0;j<i;j++){
      scalar=0;/*scalar product*/
      for(k=0;k<this->dim;k++)				
	scalar=scalar+R[i][k]*R[j][k];			
      for(k=0;k<this->dim;k++)			
	line[k]=line[k]+scalar*R[j][k];			
    }
    for(k=0;k<this->dim;k++)		
      R[i][k]=R[i][k]-line[k];
    
    /*third step*/
    norm=0;
    for(k=0;k<this->dim;k++)			
      norm=norm+R[i][k]*R[i][k];
    
    norm=sqrt(norm);
    for(k=0;k<this->dim;k++)		
      R[i][k]=R[i][k]/norm;					
  }
//   /*We generate the transpose of R*/
//   for(i=0;i<this->dim;i++) 
//     for(j=0;j<this->dim;j++)
//       RT[i][j]=R[j][i]; 
}

void MultidimBenchmarkFF::rotate_x_given_R(double *res_x_rotated, double *x)
{
    int i, j;
    const int const_dim = this->dim;

    if (x == res_x_rotated) // create temporal array, if result and x are the same
    {
        double tmp[const_dim];
        for (i = 0; i < const_dim; i++)
        {
            tmp[i] = 0;
            for (j = 0; j < const_dim; j++)
                tmp[i] = tmp[i] + R[i][j] * x[j];
        }
        for (i = 0; i < const_dim; i++)
            x[i] = tmp[i];
    }
    else
    {
        for (i = 0; i < const_dim; i++)
        {
            res_x_rotated[i] = 0;
        }
        for (i = 0; i < const_dim; i++)
        {
            for (j = 0; j < const_dim; j++)
                res_x_rotated[i] += R[i][j] * x[j];
        }
    }
}

MultidimBenchmarkFF *load_problem(int problem_index, int dim, double x_lower_lim, double x_upper_lim)
{   
    if (problem_index == 11)
    {
        std::cout << "ERROR: Problem index 11 requires seed for randomly generated instance." << endl;
        exit(1);
    }
    
    return load_problem(problem_index, dim, x_lower_lim, x_upper_lim, 2, false);
}


MultidimBenchmarkFF *load_problem(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE)
{
    MultidimBenchmarkFF *problem;
    switch (problem_index)
    {
    case 1:
        problem = new F1(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 2:
        problem = new F2(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 3:
        problem = new F3(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 4:
        problem = new F4(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 5:
        problem = new F5(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 6:
        problem = new F6(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 7:
        problem = new F7(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 8:
        problem = new F8(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 9:
        problem = new F9(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 10:
        problem = new F10(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    case 11:
        problem = new F11(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    default:
        cout << "Incorrect problem index, only integers between 1 and 8 allowed. problem_index = " << problem_index << "  was provided." << endl;
        std::exit(1);
        break;
    }
    return problem;
}



