/*
  Multimodal function generator                      
  
  Header file containing other than public interface stuff related to generator.c.
  
  Author: 
  
  Jani Rönkkönen						
  Lappeenranta University of Technology			
  Department of Information Technology				
  P.O.Box 20, FIN-53851 LAPPEENRANTA, Finland			
  E-mail: jani.ronkkonen@lut.fi	
  
  Copyright (c) 2007 Jani Rönkkönen
  
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

#ifndef _GEN_PRIVATE_H_
#define _GEN_PRIVATE_H_

/*Structure g_function_pointers links the functions of the public interface (generator.c)
  to implementations of correct function family.*/
struct g_function_pointers
{
  void (*ptr_initialize)(FILE *init_file);
  double (*ptr_calculate)(double *x); 
  double (*ptr_safe_calculate)(const double *x);
  int (*ptr_check_result)(struct g_pop_data pdata);
  void (*ptr_get_initialization_point)(double *x);
  const double* (*ptr_constraint_function)(int number);
  int (*ptr_constraint_amount)(void);
  const double* (*ptr_lower_constraints)(void);
  const double* (*ptr_upper_constraints)(void);
  void (*ptr_function_data)(void);
  int (*ptr_global_amount)(void); 
  int (*ptr_local_amount)(void); 
  const double* (*ptr_optima_location)(int number);
  double (*ptr_optima_value)(void);
  int (*ptr_check_lresult)(struct g_pop_data pdata);
  const double* (*ptr_loptima_location)(int number);
};

/*Returns the value of function_family tag*/
int g_function_family(void);

/*Returns the value of output_level tag*/
int g_output_level(void);

/*A function to re-set dimension after initialization*/
void g_set_dimension (int dimension);

#endif
