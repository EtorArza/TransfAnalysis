/*
  Multimodal function generator                      
  
  The general framework.
  
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

#include <stdlib.h>
#include <stdio.h>

#include "quadratic.h"
#include "gen_private.h"
#include "utilities.h"

/*The amount of parameters common to all function families which are 
  read from initialization file*/
#define COMMONPARAM 4

static int function_family=0; /*The used function family*/
static int output_level=0;
static int D=0; /*The dimension of the problem*/
static int random_seed; /*The seed used to initialize the random number generator*/
static int feval=0; /*keeps track of the function evaluations.*/

/*Struct containing the function pointers to implementations of the 
  functions on the public interface*/
static struct g_function_pointers fptr={NULL};

/*Array of sets of function pointers among which to select the correct
  based on the function family tag. When a new function family is added,
  to the generator, an entry needs to be added here to appropriate 
  set_functions function*/
static void (*func_table[])(struct g_function_pointers *fptr)={
  &quad_set_functions,
  NULL,
  NULL,
  NULL,
  NULL
};

/*If generator is not initialized displays an error message an exits the program*/
void g_check_status(void)
{	
  if(fptr.ptr_initialize==NULL)
    { 
      fprintf(stderr,"Error: Generator not initialized.\n");
      exit(-1);
    }
}

int g_function_family(void)
{
  return function_family;
}

int g_output_level(void)
{
  return output_level;
}

int g_get_evaluations(void)
{
  return feval;
}

void g_reset_evaluations()
{
  feval=0;
}

int g_dimension(void)
{
  return D;
}

void g_set_dimension (int dimension)
{
  if (dimension != D && output_level > 1)
    printf ("Warning: Function has a fixed dimension D=%d.\n", dimension);
  D = dimension;
}

/*Attach the appropriate set of functions to the function pointers in fptr
  (defined by function_family tag), so we get to use the right function family*/
void g_set_functions()
{

    if(func_table[function_family-1]==NULL){
      fprintf(stderr,"Error: Inappropriate value for function family: %d\n",function_family);
      exit(-1);
    }
  func_table[function_family-1](&fptr);
}

/*The initialization data is checked and stored*/
void g_do_initialization(char *filename, int seed, int dim)
{
  int i;
  char init_data[4][20];
  FILE *init_file;
  
  feval=0;
  
  init_file=g_file_open(filename,"r");
  
  for (i=0;i<COMMONPARAM;i++){
    if(fgets(init_data[i],20,init_file)==NULL){
      fprintf(stderr,"Error: Too few parameters in initialization file\n");
      exit(-1);
    }	                  
  }
  
  if(atoi(init_data[0])>=0){
    function_family=atoi(init_data[0]);
  }else{ 
    fprintf(stderr,"Error: Inappropriate value for function family: %d\n",function_family);
    exit(-1);
  } 
  
  /*Output level*/	
  if(atoi(init_data[1])<1 || atoi(init_data[1])>3){
    printf("Using default output_level=3\n");
    output_level=3;
  }else{
    output_level=atoi(init_data[1]);
  }
  
  /*Dimension*/
  D=dim;
  if(dim<1 || dim>DMAX){
      printf("Unsuitable dimension, D = ", dim, "exiting... \n");
      exit(1);
  }
  
  /*The seed for the random number generator*/
  random_seed=atoi(init_data[3]);
  if(seed!=666666666)
    random_seed=seed;
  g_init_seed(random_seed);/*Initialize the generator*/
  g_set_functions();
  fptr.ptr_initialize(init_file);
  fclose(init_file);  
  
  if(output_level>1){
    printf("\nThe generator was succesfully initialized with values:\n");
    g_function_data();

    /*Initialize the generator with random value, so g_get_initialization_point won't always give same points for same function*/
    g_init_seed(-1);
  }  
}

void g_initialize(char *filename, int dim)
{
  g_do_initialization(filename,666666666, dim); 
}
void g_seeded_initialize(char *filename,int seed, int dim)
{
  g_do_initialization(filename,seed, dim);
}

double g_calculate(double *x)
{
  g_check_pointer(x);
  g_check_status();
  feval++;
  return fptr.ptr_calculate(x);
}

double g_dummy_calculate(double *x)
{
  g_check_pointer(x);
  g_check_status();
  return fptr.ptr_calculate(x);
}

double g_safe_calculate(const double *x)
{
  g_check_pointer(x);
  g_check_status();
  feval++;
  return fptr.ptr_safe_calculate(x);
}

double g_safe_dummy_calculate(const double *x)
{
  g_check_pointer(x);
  g_check_status();
  return fptr.ptr_safe_calculate(x);
}

int g_check_result(struct g_pop_data pdata)
{
  g_check_pointer(pdata.population);
  g_check_status();
  return fptr.ptr_check_result(pdata);
}

int g_check_lresult(struct g_pop_data pdata)
{
  g_check_pointer(pdata.population);
  g_check_status();
  return fptr.ptr_check_lresult(pdata);
}

void g_get_initialization_point(double *x)
{
  g_check_pointer(x);
  g_check_status();
  fptr.ptr_get_initialization_point(x);
}

const double* g_constraint_function(int number)
{
  g_check_status();
  return fptr.ptr_constraint_function(number);
}

int g_constraint_amount(void)
{
  g_check_status();
  return fptr.ptr_constraint_amount();
}

const double* g_lower_constraints(void)
{
  g_check_status();
  return fptr.ptr_lower_constraints();
}
const double* g_upper_constraints(void)
{
  g_check_status();
  return fptr.ptr_upper_constraints();
}

void g_function_data(void)
{
  g_check_status();
  
  printf("Function family=%d\n",function_family);
  printf("Function dimension D=%d\n",D);	
  if (random_seed==-1)
    printf("Random seed: changing\n");	
  else
    printf("Random seed: %d\n",random_seed);
  
  fptr.ptr_function_data();
}

int g_global_amount(void)
{
  g_check_status();
  return fptr.ptr_global_amount();
}
 
int g_local_amount(void) 
{
  g_check_status();
  return fptr.ptr_local_amount();
}

const double* g_optima_location(int number)
{
  g_check_status();
  return fptr.ptr_optima_location(number);
}

const double* g_loptima_location(int number)
{
  g_check_status();
  return fptr.ptr_loptima_location(number);
}

double g_optima_value(void) 
{
  g_check_status();
  return fptr.ptr_optima_value();
}
