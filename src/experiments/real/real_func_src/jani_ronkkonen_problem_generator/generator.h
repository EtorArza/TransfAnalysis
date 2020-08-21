/*
  Multimodal function generator                      
  
   Header file containing the public interface of the generator. 
   Include this file to your code to use the generator module.
  
  Author: 
  
  Jani R�nkk�nen						
  Lappeenranta University of Technology			
  Department of Information Technology				
  P.O.Box 20, FIN-53851 LAPPEENRANTA, Finland			
  E-mail: jani.ronkkonen@lut.fi	
  
  Copyright (c) 2007 Jani R�nkk�nen
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.			
*/

#ifndef _GENERATOR_H_
#define _GENERATOR_H_


#if defined (__cplusplus)
extern "C" {
#endif

/*
 * Put plain C function declarations here ...
 */ 


/**************
* Definitions *
**************/

/*Structure popdata is used to define the parameter array for g_check_result (and g_check_lresult) function.*/
struct g_pop_data
{
  /*Pointer to beginning of 2 dimensional array (or dynamic array with similarly 
    arranged data) containing the solutions we want to send to check_result.*/
  double *population; 
  /*Population size, how many solutions there are in the array (defines the
    first dimension of the array). It does not matter if you have an array
    which is bigger than the data, check_result will only use pop_size first values 
    (starting from 0)*/
  int pop_size;
  /*The second dimension of the population array (the dimension of the problem). Use 
    the actual dimension of the array. In check_result only D first values will be used, 
    but if this value does not match the actual dimension of the original array, the 
    indexing inside check_result will not work correctly*/
  int max_Dsize;
  /* Defines the accuracy the minima should be found (objective function value). 
     The value should be rather small.
     NOTE: Avoid using values smaller than 0.0000001 especially with common family,
     to prevent rounding errors from becoming an issue.*/
  double accuracy;	
};

/*********************
* The main functions *
*********************/

/*Can be used to change the function generator is using 
  Parameters:
  Filename is the name of the file the initialization data should be read.*/
void g_initialize(char *filename, int dim); 


/*Works as g_initialize, but uses the given seed to initialize random number generator
  instead of reading it from the initialization file. Special: value seed=666666666
  causes the same effect as calling the g_initialize (seed is red from initialization file).*/
void g_seeded_initialize(char *filename, int seed, int dim);


/*Returns the value for current function in point defined by vector x and increases the amount 
  of function evaluations by 1 (use g_get_evaluations() to get the value).
  Parameters:
  x is a length D array of double numbers. If a parameter of x is outside the box constraints 
  defined in current function, it is fixed by mirroring the value from 
  the violated boundary, so function g_calculate may modify parameter x.*/
double g_calculate(double *x); 

/*Works as g_calculate, but does not increase the counter for function evaluations.*/
double g_dummy_calculate(double *x);


/*******************
* Helper functions *
*******************/

/*Works as g_calculate, but does not modify parameter x (still uses mirroring to define the proper 
  location inside boundaries, where the function value is calculated). May be usefull for example
  when using gradient based methods.*/
double g_safe_calculate(const double *x);


/*Works as g_safe_calculate, but does not increase the counter for function evaluations.*/
double g_safe_dummy_calculate(const double *x);


/*Returns the amount of different global minima the parameter "population" contains. Can be used to 
  determine how many of the global minima a algorithm was able to find. The total amount of global 
  minima the current function contains can be received with function global_amount
  Parameters:  
  Structure g_pop_data defines the array of solutions in 2 dimensional array of form:
  population[pop_size][max_Dsize] (actually a single dimensional array with pop_size 
  amount of max_Dsize sized datablocks in a row, so you may replace the static array 
  with a dynamic one with data arranged accordingly).

  Example: we have a population array in the main program called pop: 
  
  double pop[100][10];
  struct g_pop_data pdata;
  int result;
  
  pdata.population=pop[0];
  pdata.pop_size=50;
  pdata.max_Dsize=10;
  pdata.accuracy=0.001;
  result=g_check_result(pdata);
  
  This would store to variable result the amount of minima defined by the 50 first
  members of pop (elements pop[0] to pop[49]). NOTE that the max_Dsize must be the dimension of the 
  original array, not the dimension of the current problem (D), allthough they may be same. Inside
  check_result only D first dimensions are used, but the indexing will not work, if we don't know
  the exact second dimension of the original array, which may be larger than D.*/
int g_check_result(struct g_pop_data pdata);


/*Initializes x a uniform random value inside the box constraints of current function.
  Parameters:
  x is a length D array of double numbers.*/
void g_get_initialization_point(double *x);


/*Can be used to receive the multipliers for constraint functions. Returns a pointer to
  a D+1 dimensional array which contains the multipliers a_i for one constraint function in form:
  a_0*x_0+a_1*x_1+...+a_D-1*x_D-1+a_D<=0. The return value is constant 
  for a reason: If the reveived pointer is regardless used to change the values, the 
  changes will be made to the array inside the generator module and the 
  function will return erronous values until initialize is called again or the program 
  is restarted. For unrotated cases, g_upper_constraints and g_lower_constraints can be
  used for acquiring the box constraints in simpler form.
  Parameters:
  The number parameter (in range [0,g_constraint_amount[) defines, which constraint to return. 
  Each number returns a different constraint.*/ 
const double* g_constraint_function(int number);

/*Returns the amount of constraint functions retriavable by g_constraint_function.*/
int g_constraint_amount(void);


/*Returns a pointer to D dimensional array containing the lower limit of the box 
  constraint for that dimension for unrotated function. The return value is constant 
  for a reason: If the reveived pointer is regardless used to change the values, the 
  changes will be made to the array inside the generator module and the constraints 
  will be erronously handled until initialize is called again or the program is restarted.

  NOTE: the value is for unrotated case, and if the whole function is rotated (In 
  quadratic family, only the minima are rotated and the box constraints are fine), also 
  the box constraint gets rotated and becomes a linear constraint function. The constraints
  are still handled as box constraints inside the generator, because the rotated 
  function is unrotated for constraint handling, but this value is not suitable to be 
  used as a limit for initializing population for rotated functions (use 
  g_get_initialization_point instead). Rather these values can be used (with the upper limits)
  to determine the range of box constraints for example for scaling purposes. The constraint 
  functions can be acquired with g_constraint_function.*/
const double* g_lower_constraints(void);


/*Returns a pointer to D dimensional array containing the upper limit of the box 
  constraint for that dimension for unrotated function. The return value is constant 
  for a reason: If the reveived pointer is regardless used to change the values, the 
  changes will be made to the array inside the generator module and the constraints 
  will be erronously handled until initialize is called again or the program is restarted.

  NOTE: the value is for unrotated case, and if the whole function is rotated (In 
  quadratic family, only the minima are rotated and the box constraints are fine), also 
  the box constraint gets rotated and becomes a linear constraint function. The constraints
  are still handled as box constraints inside the generator, because the rotated 
  function is unrotated for constraint handling, but this value is not suitable to be 
  used as a limit for initializing population for rotated functions (use 
  g_get_initialization_point instead). Rather these values can be used (with the lower limits)
  to determine the range of box constraints for example for scaling purposes. The constraint 
  functions can be acquired with g_constraint_function.*/
const double* g_upper_constraints(void);


/*Prints on screen the essential data of the current function*/
void g_function_data(void);


/*Returns the dimension of the current function*/
int g_dimension(void);


/*Returns the amount of global minima in the current function*/
int g_global_amount(void); 


/*Returns the amount of local minima in the current function*/
int g_local_amount(void); 


/*Can be used to receive the coordinates for the global minima. Returns a pointer to
  a D dimensional array which contains one of the minima. The return value is constant 
  for a reason: If the reveived pointer is regardless used to change the values, the 
  changes will be made to the minima array inside the generator module and the 
  optima_location function will return erronous coordinates until initialize is called 
  again or the program is restarted.
  Parameters:
  The number parameter (in range [0,number of global minima[) identifies an optimum. 
  Each number returns a different optimum.
  
  NOTE for cosine function family: If G[i]= 1, i:th dimension will be flat and any value for 
  that dimension will yield an optimum (the optimum is a valley shape, not one point). In this 
  case local_amount will return 0 for that dimension (Of course 0 may also be a real coordinate).  
  The check_result  function basically ingnores a dimension with G=1, so any valua for that will be 
  treated optimum.*/
const double* g_optima_location(int number);


/*Returns the number of function evalutions performed this far. The number is increased
  each time g_calculate is called and is reset to 0 when g_initialize or 
  g_reset_evaluations is called or when the generator is restarted.*/
int g_get_evaluations(void);


/*Resets the function evaluations calculator to 0*/
void g_reset_evaluations(void);


/*Returns the value of global minima*/
double g_optima_value(void); 


/****************************************************
* Functions that only work for some of the families *
****************************************************/

/*Can be used to receive  the coordinates for the global minima. Returns a pointer to
  a D dimensional array which contains one of the minima. The return value is constant 
  for a reason: If the reveived pointer is regardless used to change the values, the 
  changes will be made to the minima array inside the generator module and the 
  optima_location function will return erronous coordinates until initialize is called 
  again or the program is restarted.
  Parameters:
  The number parameter (in range [0,number of localal minima[) identifies an optimum. 
  Each number returns a different optimum. 

  NOTE that this functionality has not been implemented for all functions the generator is able to produce. 
  The program exits, if this function is called for a function for which the functionality has not been 
  implemented or the function in guestion does not contain any local optima.*/
const double* g_loptima_location(int number);


/*Returns the amount of different local minima, which are not global the parameter "population" contains.
  Can be used to determine how many of the local minima a algorithm was able to find. The total amount of 
  local minima the current function contains can be received with function local_amount
  Parameters:  
  Structure g_pop_data defines the array of solutions in 2 dimensional array of form:
  population[pop_size][max_Dsize] (actually a single dimensional array with pop_size 
  amount of max_Dsize sized datablocks in a row, so you may replace the static array 
  with a dynamic one with data arranged accordingly).

  Example: we have a population array in the main program called pop: 
  
  double pop[100][10];
  struct g_pop_data pdata;
  int result;
  
  pdata.population=pop[0];
  pdata.pop_size=50;
  pdata.max_Dsize=10;
  pdata.accuracy=0.001;
  result=g_check_lresult(pdata);
  
  This would store to variable result the amount of local minima defined by the 50 first
  members of pop (elements pop[0] to pop[49]). NOTE that the max_Dsize must be the dimension of the 
  original array, not the dimension of the current problem (D), allthough they may be same. Inside
  check_result only D first dimensions are used, but the indexing will not work, if we don't know
  the exact second dimension of the original array, which may be larger than D.

  NOTE that this functionality has not been implemented for all functions the generator is able to produce.
  The program exits, if this function is called for a function for which the functionality has not been 
  implemented or the function in guestion does not contain any local optima.
  
  NOTE pdata.accuracy should be smaller than the difference between the value of global optima and the deepest local 
  optimum. Otherwise g_check_lresult may confuse some global optima to local optima.*/
int g_check_lresult(struct g_pop_data pdata);


#if defined (__cplusplus)
}

#endif


#endif
