#include "Tools.h"
#include <math.h>
#include <assert.h>
#include "MultidimBenchmarkFF.h"
#include "generator.h"
#include <omp.h>
#include <unistd.h>
#include "constants.h"

MultidimBenchmarkFF::MultidimBenchmarkFF(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE)
{
    this->dim = dim;
    rng = new RandomNumberGenerator();
    double delta1;
    double delta2;
    temp_vect_1 = new double[this->dim];


    if (SEED == -1)
    {
        rng->seed(2);
        double delta1 = 0;
        double delta2 = 0;
    }
    else
    {
    rng->seed(SEED);
    double delta1 = rng->random_0_1_double() / 10.0;
    double delta2 = rng->random_0_1_double() / 10.0;
    }    

    

    this->x_lower_lim = x_lower_lim + delta1 * (x_upper_lim - x_lower_lim);
    this->x_upper_lim = x_upper_lim - delta2 * (x_upper_lim - x_lower_lim);

    // cout << " seed = " << SEED << ", lims = " <<  "(" << this->x_lower_lim << ", " << this->x_upper_lim << ")" << ", delta = " <<  "(" << delta1 << ", " << delta2 << ")" << endl;

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
        rotate_x_given_R(local_tmp_vec, x_vec_0_1);
    }else
    {
        copy_array(local_tmp_vec, x_vec_0_1, this->dim);
    }
    



    for (int i = 0; i < this->dim; i++)
    {   
        // assert(x_vec_0_1[i] > -0.0000001);
        // assert(x_vec_0_1[i] < 1.0000001);
        local_tmp_vec[i] = this->get_x_lim_lower() + min(1.0,max(0.0,x_vec_0_1[i])) * (this->get_x_lim_upper() - this->get_x_lim_lower());
    }







    
    // Maximization is assumed in MultidimBenchmarkFF problems.
    return this->FitnessFunc(local_tmp_vec); 
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



// Sphere (bowl-shaped) [-5.12,5.12]
double F1::FitnessFunc(double* x_vec){
    long double res = 0;
    for (int i = 0; i < dim; i++)
    {
        res += (long double) x_vec[i] * (long double) x_vec[i];
    }
    return -res;
}


// ROTATED HYPER-ELLIPSOID (bowl-shaped) [-65.536, 65.536]
double F2::FitnessFunc(double* x_vec){
    long double res = 0;
    long double sum_of_x_j = 0;
    for (int i = 0; i < dim; i++)
    {   
        sum_of_x_j += (long double) x_vec[i] * (long double) x_vec[i];
        res += (long double) x_vec[i] * (long double) x_vec[i];
    }
    return -res;
}


// TRID FUNCTION (bowl-shaped) [-4, 4]
double F3::FitnessFunc(double* x_vec){
    long double res = 0;
    long double sum_of_x_j = 0;

	
    long double sum1 = -2.0L;
    long double sum2 = -4.0L;
 
    for (int i = 0; i < dim; i++)
    {   
        sum1 += pow((long double) x_vec[i]-1.0L,2);
    }

    for (int i = 1; i < dim; i++)
    {   
        sum2 += (long double) x_vec[i] * (long double) x_vec[i-1];
    }
    res = sum1 - sum2;

    return - res;
}


// sum [log(x_i)^2] (bowl-shaped) [0.5, 3]
double F4::FitnessFunc(double* x_vec){
    long double res = 0;
    for (int i = 0; i < dim; i++)
    {
        res += pow(log((long double) x_vec[i]), 2);
    }
    return -res;    
}
 


// sum of different powers (bowl) [-1,1]
double F5::FitnessFunc(double* x_vec){
    long double res = 0.0L;

    for (int i = 0; i < dim; i++)
    {
        res += FastPow(abs((long double)x_vec[i]), i);
    }

    return -res;
}


// sum squares (bowl) [-10, 10]
double F6::FitnessFunc(double* x_vec){
    long double res = 0.0;

    for (int i = 0; i < dim; i++)
    {
        res += (long double) i * (long double) x_vec[i] * (long double)  x_vec[i];
    }

    return -res;
    
}


// langermann (multiopt-shaped) [0,10]
double F7::FitnessFunc(double* x_vec){
    long double res = 0.0L;
    long double A[5][2] = {{3.0L,5.0L},{5.0L,2.0L},{2.0L,1.0L},{1.0L,4.0L},{7.0L,9.0L}};
    long double c[5]  = {1.0L,2.0L,5.0L,2.0L,3.0L};
    const long double PI_L = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482L;
    const long double one_DIV_PI_L = 0.3183098861837906715377675267450287240689192914809128974953346881177935952684530701802276055325L;
    for (size_t i = 0; i < 5; i++)
    { 
        long double sum_i = 0.0L;

        for (int j = 0; j < dim; j++)
        {
            sum_i += pow((long double) x_vec[j] - A[i][j % 2], 2);
        }
        res += c[i] * exp(- one_DIV_PI_L * sum_i) * cos(PI_L * sum_i);
    }
    return  - res;    
}

// SCHWEFEL FUNCTION (multiopt) [-500,500]
double F8::FitnessFunc(double* x_vec){
    long double res = 0;

    for (size_t i = 0; i < dim; i++)
    {
        res += (long double) x_vec[i] * sin(sqrt(abs(x_vec[i])));
    }
    

   return - res;
}




// Rastrigin (many local optima) [-5.12, 5.12]
double F9::FitnessFunc(double* x_vec){
    long double res = 10.0L * dim;

    for (int i = 0; i < dim; i++)
    {
        res += (long double) x_vec[i] * (long double) x_vec[i] - 10.0L * cos(2.0L * M_PIl * (long double) x_vec[i]);
    }

    return -res;
}


// Levy (many local optima) [-10, 10]
double F10::FitnessFunc(double* x_vec){
    long double w_1 = 1.0L + ((long double) x_vec[0] - 1.0L)/4.0L;
    long double res = sin(M_PIl * w_1) * sin(M_PIl * w_1);

    for (int i = 0; i < dim - 1; i++)
    {
        long double w_i = 1.0L + ((long double) x_vec[i] - 1.0L)/4.0L;
        res += (w_i - 1.0L) *(w_i - 1.0L) * (1.0L + 10.0L*FastPow(sin(M_PIl * w_i + 1.0),2));
    }

    long double w_d = 1.0L + ((long double) x_vec[dim - 1] - 1.0L)/4.0L;

    res += (w_d - 1.0L) * (w_d - 1.0L) * (1.0 + FastPow(sin(2.0 * M_PIl * w_d), 2.0));

    return -res;
}



// Griewank (many local optima) [-600,600]
double F11::FitnessFunc(double* x_vec){
    long double val_1 = 0.0;
    for (int i = 0; i < dim; i++)
    {
        val_1 += (long double) x_vec[i] * (long double) x_vec[i] / 4000.0L;
    }

    long double val_2 = 1.0;

    for (int i = 1; i <= dim; i++)
    {
        val_2 *= cos((long double) x_vec[i-1] / sqrt((long double) i));
    }

    long double res = 1.0 + (long double)val_1 - (long double)val_2;

    return -res;
}




// Ackley (many local optima) [-32.768, 32.768]
double F12::FitnessFunc(double* x_vec){
    long double res = 0.0L;
    long double sum_1 = 0.0L;
    long double sum_2 = 0.0L;

    for (int i = 0; i < dim; i++)
    {
        sum_1 += (long double)x_vec[i] * (long double)x_vec[i];
    }

    sum_1 = -20.0L * exp(-0.2L * sqrt(sum_1 / (long double) dim));

    for (int i = 0; i < dim; i++)
    {
        sum_2 += cos(2.0 * M_PIl * (long double)x_vec[i]);
    }

    sum_2 = -exp(sum_2 / (long double) dim);

    res = sum_1 + sum_2;
    return -res;
}



static int current_jobs_with_this_seed = 0;
FRandomlyGenerated::FRandomlyGenerated(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE)  : MultidimBenchmarkFF(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE)
{
    char config_path[] = "src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat";

    SEED = max(2,SEED - SEED % use_same_seed_every);

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


FRandomlyGenerated::~FRandomlyGenerated()
{
    #pragma omp critical(adshfafoiadfoak)
    {
        //cout << current_seed << "," << dim << ")" << endl;
        current_jobs_with_this_seed--;
    }
};


double FRandomlyGenerated::FitnessFunc(double* x_vec)
{
    return g_calculate(x_vec);
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





MultidimBenchmarkFF* load_problem_with_default_lims(int problem_index, int dim,  int SEED, bool ROTATE)
{

    // 1)  -> Sphere (bowl-shaped) [-5.12,5.12]
    // 2)  -> ROTATED HYPER-ELLIPSOID (bowl-shaped) [-65.536, 65.536]
    // 3)  -> TRID FUNCTION (bowl-shaped) [-4.0, 4.0]
    // 4)  -> sum(log(x_i)^2) (bowl-shaped) [0.5, 3.0]
    // 5)  -> sum of different powers (bowl-shaped) [-1.0,1.0]
    // 6)  -> sum squares (bowl-shaped) [-10.0, 10.0]
    // 7)  -> langermann (multiopt-shaped) [0.0, 10.0]
    // 8)  -> SCHWEFEL (multiopt-shaped) [-500.0,500.0]
    // 9) -> Rastrigin (many local optima) [-5.12, 5.12]
    // 10) -> Levy (many local optima) [-10.0, 10.0]
    // 11) -> Griewank (many local optima) [-600.0,600.0]
    // 12) -> Ackley (many local optima) [-32.768, 32.768]
    double x_lower_lim;
    double x_upper_lim;

    switch (problem_index)
    {
    case 0:
        x_lower_lim = 0;
        x_upper_lim = 1;
        break;
    case 1:
        x_lower_lim = -5.12;
        x_upper_lim = 5.12;
        break;
    case 2:
        x_lower_lim = -65.536;
        x_upper_lim = 65.536;
        break;
    case 3:
        x_lower_lim = -4.0;
        x_upper_lim = 4.0;
        break;
    case 4:
        x_lower_lim = 0.5;
        x_upper_lim = 3.0;
        break;
    case 5:
        x_lower_lim = -1.0;
        x_upper_lim = 1.0;
        break;
    case 6:
        x_lower_lim = -10.0;
        x_upper_lim = 10.0;
        break;
    case 7:
        x_lower_lim = 0.0;
        x_upper_lim = 10.0;
        break;
    case 8:
        x_lower_lim = -500.0;
        x_upper_lim = 500.0;
        break;
    case 9:
        x_lower_lim = -5.12;
        x_upper_lim = 5.12;
        break;
    case 10:
        x_lower_lim = -10.0;
        x_upper_lim = 10.0;
        break;
    case 11:
        x_lower_lim = -600.0;
        x_upper_lim = 600.0;
        break;
    case 12:
        x_lower_lim = -32.768;
        x_upper_lim = 32.768;
        break;
    default:
        cout << "Incorrect problem index, only integers between 1 and 16 allowed. problem_index = " << problem_index << "  was provided." << endl;
        std::exit(1);
        break;
    }


    return load_problem(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);


}



MultidimBenchmarkFF *load_problem(int problem_index, int dim, double x_lower_lim, double x_upper_lim, int SEED, bool ROTATE)
{
    MultidimBenchmarkFF *problem;
    switch (problem_index)
    {
    case 0:
        problem = new FRandomlyGenerated(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
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
    case 12:
        problem = new F12(problem_index, dim, x_lower_lim, x_upper_lim, SEED, ROTATE);
        break;
    default:
        cout << "Incorrect problem index, only integers between 0 and 16 allowed. problem_index = " << problem_index << "  was provided." << endl;
        std::exit(1);
        break;
    }
    return problem;
}



