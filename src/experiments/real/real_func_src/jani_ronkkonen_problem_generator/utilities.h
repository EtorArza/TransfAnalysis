/*
  Multimodal function generator                      
  
  Header file containing the utility functions used by multiple 
  function families. 
  
  Author: 
  
  Jani Rönkkönen						
  Lappeenranta University of Technology			
  Department of Information Technology				
  P.O.Box 20, FIN-53851 LAPPEENRANTA, Finland			
  E-mail: jani.ronkkonen@lut.fi	
  
  Copyright (c) 2007 Jani Rönkkönen, Ville Kyrki, Jarmo Ilonen
  
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

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <stdio.h>

/*Used instead of M_PI to keep code ansi compatible*/
#define PII 3.14159265358979323846264338327950288419716939937510f 
#define SMALL 1.0e-9 /*A small number*/
#define DMAX 100 /*The maximum dimension of the problem*/

/* rng by Park&Miller's method with Schrage's mod-multiplication.
   Returns a uniform random number on interval (0,1) (exclusive end points).
   Modified from implementation by Ville Kyrki*/
double g_randu(void);

/*Uses srand to seed the random number generator (g_randu). -1 takes the seed from time.
  Modified from implementation by Ville Kyrki */
void g_init_seed(int random_seed);

/*Returns gaussian random numbers ( N(0,1) ) using Carter's polar form of 
  Box-Muller transformation.*/
double g_randn(void);

/*Calculates the euclidean distance between vectors a and b of length D*/
double g_euclidean(const double *a, const double *b, int D);

/*Opens a file defined by parameter "name" in mode "method" and handles errors*/
FILE* g_file_open(char *name, char *method);

/*Constraint handling: Mirroring, the value is reflected back from the violated 
  boundary. Modifies vector x when necessary llimit and ulimit are arrays of 
  length D, containing the box constraints*/
void g_constraints(double *x, int D, double const *llimit, const double *ulimit);

/*Returns the factorial of n*/
int g_factorial(int n);

/*If ptr is NULL displays an error message an exits the program*/
void g_check_pointer(const void *ptr);

/* Read the bezier lines from the initialization file*/
void g_bezier_initialize(FILE *init_file);

/*Uses the defined Bezier curves to relocate point coord to corresponding place in the stretched 
  function. Note that function modifies array coord, so the original values are lost.*/
void g_bezier_shift(double *coord, const double *llimit, const double *ulimit);

/*Inverse Bezier shift, llimit and ulimit are arrays of length D, containing the box constraints*/
void g_inverse_bezier_shift(double *coord, const double *llimit, const double *ulimit);

/*Print the bezier data*/
void g_bezier_data(void);

/*Produces a new random transformation matrix to array A and its transpose to array AT 
  with algorithm presented by Niklaus Hansen and Andreas Ostmeier in: Completely 
  Derandomized Self-Adaptation in Evolution Strategies, Evolutionary computation 9(2),2001*/
void g_init_transform(void);

/*Unrotates the given vector x with transpose of A*/
void g_unrotate(double *x);

/*Rotates the given vector x with transpose of A, used to rotate constraint multiplier values*/
void g_rotate_multipliers(double *x);

/*Rotates the given vector x with A*/
void g_rotate(double *x);

/*Rotates the target covariance matrix using: A*target*AT. target is supposed to be
a 2 dimensional array defined as target[DMAX][DMAX].*/
void g_cov_rotate(double *target);

/*Calculates the ordinary matrix product FS and stores the result to R. F, S and R are
 supposed to be 2 dimensional arrays defined as F[DMAX][DMAX], but only D first values 
 are used in the calculation (R[D][D]=F[D][D]S[D][D]).*/
void g_matrix_multiply(double *F, double *S, double *R);

/*Calculates the inverse of orig using Gaussian elimination and stores the result to 
  inv. orig and inv are supposed to be 2 dimensional arrays defined as orig[DMAX][DMAX] 
  but only D first values are used in the calculations.
  Modified from implementation by Jarmo Ilonen*/
void g_matrix_inversion(double *orig, double *inv);

/*Initializes the 2 dimensional constr array, which includes the multipliers for constraint functions.
  llimit and ulimit are arrays of length D, containing the box constraints and rotation is a flag indicating 
  is the function rotated or not, 1 means rotated, 0 unrotated.*/
void g_init_constraint_functions(const double *llimit, const double *ulimit, int rotation);

/*Returns one of the 2*D vectors containing multipliers for constraint functions. Vectors are D+1 dimensional. 
  Number parameter defines the constraint and different number returns different constraint.*/
const double* g_get_constraint_function(int number);

#endif

