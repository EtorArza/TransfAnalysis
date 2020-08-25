/*
  Multimodal function generator                      
  
  Implementations for the quadratic function family.
  
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
#include <string.h>
#include <math.h>

#include "quadratic.h"
#include "generator.h"
#include "gen_private.h"
#include "utilities.h"

#define INITPARAM 7 /*The amount of parameters read from initialization file excluding the 
		      common parameters already red in generator.c*/
#define TRYLIMIT 10000 /*The amount of attempts to generate random coordinates for an optima
		      in initialization before deciding to give up*/
#define MINDISTLOW 0.0001 /*Lower bound for mindist*/
#define MINDISTUP 0.95*pow(quad_global_amount(),-1.0/g_dimension()) /*Upper bound for mindist*/
#define MINSHAPE 0.0001 /*The lower limit for the distribution in shape of a single optima*/
#define MAXSHAPE 1.0/sqrt(quad_local_amount()+1) /*The upper limit for the distribution 
						 in shape of a single optima*/
#define MINIMA 0.1 /*The global optimum value*/
#define DIVUPL MINIMA/8.1 /*Defines the upper limit for the optimum value of local minima as the 
			    fraction of global minima*/
#define DIVLOWL MINIMA/1.01 /*Defines the lower limit for the optimum value of local minima as the 
			     fraction of global minima*/

struct quad_optimum{/*Struct defining one optimum*/
  double optimum_value; /*The optimum value*/
  double optimum_coord[DMAX]; /*The coordinates of optimum*/
  double shape[DMAX][DMAX]; /*The (possibly rotated) covariance matrix*/
};

static double llimit[DMAX]; /*Lower limit for values of x for each dimension (box constraint)*/
static double ulimit[DMAX]; /*upper limit for values of x for each dimension (box constraint)*/
static int shape_flag; /*Flag indicating the shape of the optima: 0 means spherical, 1 random 
			 ellipsoidal and 2 randomly rotated random ellipsoidal*/
static double mindist=0.0; /*The minimum Euclidean distance between two global optima*/
static double minldist=10.0; /*The minimum Euclidean distance between two local optima*/
static int global_optima=0; /*The amount of global optima*/
static int local_optima=0; /*The amount of local optima*/
static double local_ob[2]; /*Limits for the objective function value of local optima*/
static double global_shape[2]; /*Limits for the shape of global optima*/
static double local_shape[2]; /*Limits for the shape of local optima*/
static struct quad_optimum *goptima=NULL; /*Stores the global optima*/
static struct quad_optimum *loptima=NULL; /*Stores the local optima*/

/*Implementations for the functions defined in public interface for quadratic family*/
void quad_initialize(FILE *init_file);
double quad_calculate(double *x);
double quad_safe_calculate(const double *x);
int quad_check_result(struct g_pop_data pdata);
void quad_get_initialization_point(double *x);
const double* quad_constraint_function(int number);
int quad_constraint_amount(void);
const double* quad_lower_constraints(void);
const double* quad_upper_constraints(void);
void quad_function_data(void);
int quad_global_amount(void); 
int quad_local_amount(void); 
const double* quad_optima_location(int number);
double quad_optima_value(void);
int quad_check_lresult(struct g_pop_data pdata);
const double* quad_loptima_location(int number);

void quad_set_functions(struct g_function_pointers *fptr)
{
  fptr->ptr_initialize=&quad_initialize;
  fptr->ptr_calculate=&quad_calculate;
  fptr->ptr_safe_calculate=&quad_safe_calculate;
  fptr->ptr_check_result=&quad_check_result;
  fptr->ptr_get_initialization_point=&quad_get_initialization_point;
  fptr->ptr_constraint_function=&quad_constraint_function;
  fptr->ptr_constraint_amount=&quad_constraint_amount;
  fptr->ptr_lower_constraints=&quad_lower_constraints;
  fptr->ptr_upper_constraints=&quad_upper_constraints;
  fptr->ptr_function_data=&quad_function_data;
  fptr->ptr_global_amount=&quad_global_amount; 
  fptr->ptr_local_amount=&quad_local_amount; 
  fptr->ptr_optima_location=&quad_optima_location;
  fptr->ptr_optima_value=&quad_optima_value;
  fptr->ptr_check_lresult=&quad_check_lresult; 
  fptr->ptr_loptima_location=&quad_loptima_location;
}

/* Stuff that is done when the program quits*/
void quad_end(void)
{
  if(goptima!=NULL)
    free(goptima);
  if(loptima!=NULL)
    free(loptima);
}

int quad_global_amount(void)
{	
  return global_optima;
}

int quad_local_amount(void)
{
  return local_optima;
}

void quad_function_data(void)
{ 
  if (shape_flag==2)
    printf("Optima shape: rotated\n");	
  else if(shape_flag==1)
    printf("Optima shape: ellipsoidal\n");
  else
    printf("Optima shape: spherical\n");
  printf("Number of Global optima: %d\n",quad_global_amount());
  printf("Number of Local optima: %d\n",quad_local_amount());
  printf("Minimum Euclidian distance between global optima: %g\n",mindist);
  printf("Global optima value: %g\n",MINIMA);
  printf("Optimum range for local optima=[%g,%g]\n",local_ob[0],local_ob[1]);  
  printf("Shape range for global optima=[%g,%g]\n",global_shape[0],global_shape[1]);
  printf("Shape range for local optima=[%g,%g]\n",local_shape[0],local_shape[1]);
}

const double* quad_optima_location(int number)
{
  return goptima[number].optimum_coord;
}

const double* quad_loptima_location(int number)
{
  if(g_local_amount()==0){
    fprintf(stderr,"Error: called g_loptima_location for function that does not contain local optima\n");
    exit(-1);
  } 
  return loptima[number].optimum_coord;
}


/*Compares two local optima by their minimum for qsort*/
int quad_compare(const void *arg1, const void *arg2) 
{
  struct quad_optimum *a1,*a2;
  a1=(struct quad_optimum*)arg1;
  a2=(struct quad_optimum*)arg2;
  
  if(a1->optimum_value<a2->optimum_value)
    return -1;
  if(a1->optimum_value>a2->optimum_value)
    return 1;
  return 0;
}

/*Calculates the minimum value from "amount" first minima in given point x
 from given array optima. The result is for calculating local optima and should be the result
returned by this function with goptima when called with loptima and a positive value when 
called with goptima.*/
double quad_calculate_value(double *x, int amount, struct quad_optimum *optima, double result)
{ 
  int i,j,k;
  double tmp[DMAX], diff[DMAX];
  double rtmp=0; /*Temporary storage for result*/
  
  for(i=0;i<amount;i++)  
    {
      /*If we already have smaller value than any of the local optima in the list, 
	no need to check further*/
      if(result<optima[i].optimum_value)
	return result;
	  
      for(j=0;j<g_dimension();j++)/*x-u*/
	diff[j]=x[j]-optima[i].optimum_coord[j];      

      for(j=0;j<g_dimension();j++){/*(x-u)*C*/
	  tmp[j]=0;
	  for(k=0;k<g_dimension();k++){
	    tmp[j]=tmp[j]+diff[k]*optima[i].shape[k][j];
	  }
	}
      rtmp=0;
      for(j=0;j<g_dimension();j++)/*(x-u)*C*(x-u)'*/
	rtmp=rtmp+tmp[j]*diff[j];
      
      rtmp=optima[i].optimum_value + rtmp;
      
      if(rtmp<result)
	result=rtmp;
    }  
  return result;
}

/*Ceates a shape matrix for optima*/
void quad_create_shape_matrix(double *shape, struct quad_optimum *optima)
{
  int j,k;
  double tmp=0.0;
  
  /*The covariance matrix*/
  if(shape_flag==0)
    tmp=g_randu(); /*One number for all dimensions (spherical shape)*/
  for(j=0;j<g_dimension();j++){
    for(k=0;k<g_dimension();k++){
      /*Squared random values in global_shape range for the diagonal*/
      if(j==k){
	if(shape_flag>0){ /*ellipsoidal shape*/
	  optima->shape[j][k]=g_randu()*(shape[1]-shape[0])+shape[0];
	}else{ /*spherical shape*/
	  optima->shape[j][k]=tmp*(shape[1]-shape[0])+shape[0];
	}	  
	optima->shape[j][k]=optima->shape[j][k]*optima->shape[j][k];/*^2*/
      }else{
	optima->shape[j][k]=0.0;
      }	
    }
  }  
  if(shape_flag==2)/*Rotation*/
    {
      g_init_transform();/*Create a rotation matrix*/
      g_cov_rotate(optima->shape[0]);/*Rotate the covariance matrix*/
    }
  /*Inversion*/
  g_matrix_inversion(optima->shape[0],optima->shape[0]);
}

void quad_initialize(FILE *init_file)
{
  int i,j,flag=0,counter=0;
  char init_data[INITPARAM][20]; /*All data from initialization file is first read here*/	
  char *split = NULL; /*Helper pointer for strtok*/
  double tmp=0.0;

  /*Box constraints are set to [0,1] for all dimensions*/
  for(i=0;i<DMAX;i++){
    llimit[i]=0;
    ulimit[i]=1;
  }
    
  /*The data from initialization file is red to init_data array*/  
  for (i=0;i<INITPARAM;i++){
    if(fgets(init_data[i],20,init_file)==NULL){
      fprintf(stderr,"Error: Too few parameters in initialization file\n");
      exit(-1);
    }	                  
  }
  
  /*The initialization data is checked and stored*/
  
  /*The shape of optima*/
  shape_flag=atoi(init_data[0]);
  if(shape_flag!=1 && shape_flag!=2)
    shape_flag=0; /*Optima spherical*/	

  /*Amount of global optima*/
  if(atoi(init_data[1])>0){
    global_optima=atoi(init_data[1]);
  }else{
    if(g_output_level()>1)
      printf("Unsuitable amount of global optima, using default value 1 \n");
    global_optima=1;
  }

  /*Amount of local optima*/
  if(atoi(init_data[2])>=0){
    local_optima=atoi(init_data[2]);
  }else{
    if(g_output_level()>1)
      printf("Unsuitable amount of local optima, using default value 0 \n");
    local_optima=0;
  }

  /*The minimum Euclidian distance between global optima*/
  if(atof(init_data[3])<MINDISTLOW || atof(init_data[3])>MINDISTUP){
    if(g_output_level()>1)
      printf("Unsuitable value for minimum Euclidian distance, using default value %g \n",MINDISTLOW);
    mindist=MINDISTLOW;
  }else{
    mindist=atof(init_data[3]);
  }

 /*Ranges for the objective function values of local optima*/    
  split = (char*)strtok(init_data[4], " ");
  local_ob[0]=atof(split);
  split = (char*)strtok(NULL, " ");
  if(split==NULL)
    local_ob[1]=local_ob[0];
  else
    local_ob[1]=atof(split);
 
  if(local_ob[0]>local_ob[1] || local_ob[0]<DIVLOWL || local_ob[1]>DIVUPL){
    if(g_output_level()>1)
      printf("Unsuitable value for optimum range of local optima [%g,%g]. Using defaul values: [%g,%g]\n"
	     ,local_ob[0],local_ob[1],DIVLOWL,DIVUPL);
    local_ob[0]=DIVLOWL;
    local_ob[1]=DIVUPL;    
  }
  
  /*Ranges for the global shapes*/    
  split = (char*)strtok(init_data[5], " ");
  global_shape[0]=atof(split);
  split = (char*)strtok(NULL, " ");
  if(split==NULL)
    global_shape[1]=global_shape[0];
  else
    global_shape[1]=atof(split);
  
  if(global_shape[0]>global_shape[1] || global_shape[0]<MINSHAPE || global_shape[1]>MAXSHAPE){
    if(g_output_level()>1)
      printf("Unsuitable value for shape range of global optima[%g,%g]. Using defaul values: [%g,%g]\n"
	     ,global_shape[0],global_shape[1],MINSHAPE,MAXSHAPE);
    global_shape[0]=MINSHAPE;
    global_shape[1]=MAXSHAPE;    
  }
    
  /*Ranges for the local shapes*/    
  split = (char*)strtok(init_data[6], " ");
  local_shape[0]=atof(split); 
  split = (char*)strtok(NULL, " ");
  if(split==NULL)
    local_shape[1]=local_shape[0];
  else
    local_shape[1]=atof(split);

  if(local_shape[0]>local_shape[1] || local_shape[0]<MINSHAPE || local_shape[1]>MAXSHAPE){
    if(g_output_level()>1)
      printf("Unsuitable value for shape range of local optima[%g,%g]. Using defaul values: [%g,%g]\n"
	     ,local_shape[0],local_shape[1],MINSHAPE,MAXSHAPE);
    local_shape[0]=MINSHAPE;
    local_shape[1]=MAXSHAPE;    
    }

  /*Memory allocation for goptima and loptima*/
  if(goptima==NULL){
    /*end() is defined to be performed when the program quits */
    if(atexit(quad_end)){	
      fprintf(stderr,"Error: atexit could not register function end\n");
      exit(-1);		
    }	
  }else{
    free(goptima);
    if(loptima!=NULL)
      free(loptima);
  }  
  if((goptima=malloc(g_dimension()*quad_global_amount()*
					   sizeof(struct quad_optimum)))==NULL ||
     (loptima=malloc(g_dimension()*quad_local_amount()*
					   sizeof(struct quad_optimum)))==NULL){
    fprintf(stderr,"Error: Could not allocate memory\n");
    exit(-1);
  }

  /*The global optima are generated*/
  for(i=0;i<quad_global_amount();i++){
    goptima[i].optimum_value=MINIMA; /*Optimum value*/
    counter=0;
    do{/*Coordinates*/
      /*Too mau failed attemts to place optimum, we give up*/
      if(counter==TRYLIMIT){
	fprintf(stderr,"Error: Could not fit all global optima with Euclidian distance %g\n",mindist);
	fprintf(stderr,"with %d attemps. Try again with smaller value\n",TRYLIMIT);	
	exit(-1);
      }	
      flag=1;
      for(j=0;j<g_dimension();j++)
	goptima[i].optimum_coord[j]=g_randu();      
      for(j=0;j<i;j++){ /*check the Euclidean distances*/
	if(g_euclidean(goptima[j].optimum_coord,goptima[i].optimum_coord,g_dimension())<mindist){
	  flag=0;
	  break;
	}
      }
      counter++;
    }while(flag==0);    
        
    /*The shape matrix*/
    quad_create_shape_matrix(global_shape, &goptima[i]);
  }
  
  /*The local optima are generated*/
  for(i=0;i<quad_local_amount();i++){ /*Optimum values*/
    loptima[i].optimum_value=g_randu()*(local_ob[0]-local_ob[1])+local_ob[1]; 
  }
  /*Sorting by optimum value, speeds up the generatation of local optima
    deepest minima goes to loptima[0]*/
  qsort((void*)loptima,quad_local_amount(),sizeof(loptima[0]),quad_compare);

  for(i=0;i<quad_local_amount();i++){
    counter=0;
    do{/*Coordinates*/
      /*Too mau failed attemts to place optimum, we give up*/
      if(counter==TRYLIMIT){
	fprintf(stderr,"Error: Could not fit all local optima with %d attemps.\n",TRYLIMIT);
	fprintf(stderr,"Try again with deeper local minima (current range [%g,%g])\n",
		local_ob[0],local_ob[1]);
	fprintf(stderr,"Or thinner shapes (current ranges local: [%g,%g], global: [%g,%g])\n",
		local_shape[0],local_shape[1],global_shape[0],global_shape[1]);	
	exit(-1);
      }	
      flag=1;
      for(j=0;j<g_dimension();j++)
	loptima[i].optimum_coord[j]=g_randu();      

      tmp=quad_calculate_value(loptima[i].optimum_coord,quad_global_amount(),
			       goptima,99999);
      /*Does any of the global optima mask the new local optima?*/
      if(tmp<loptima[i].optimum_value){
	flag=0;
	/*How about the already generated local optima? (the new optima can 
	  not mask others, because the local optima are in order from steepest on)*/
      }else if(quad_calculate_value(loptima[i].optimum_coord,i,loptima,tmp)<
	       loptima[i].optimum_value){
	flag=0;
      }      
      counter++;
    }while(flag==0);   
    /*The shape matrix*/
    quad_create_shape_matrix(local_shape, &loptima[i]);  
  }
  
  /*Calculate the minimum Euclidian distance between two local optima*/
 if(quad_local_amount()>1){
   minldist=HUGE_VAL;
   for(i=0;i<quad_local_amount()-1;i++){
     for(j=i+1;j<quad_local_amount();j++){
       tmp=g_euclidean(quad_loptima_location(i),quad_loptima_location(j),g_dimension());
       if(tmp<minldist)
	 minldist=tmp;      
     }
   }
 }else{
   minldist=HUGE_VAL;
 }

  /*The constraint function formulas are created. Allows the usage of g_constraints, alltough
   not very usefull with quadratic family, because functions are newer rotated.*/
  g_init_constraint_functions(llimit,ulimit,0);
}

double quad_calculate(double *x)
{
  double result=99999;
  
  g_constraints(x,g_dimension(),llimit,ulimit);/*Box constraint handling, corrects x, if necessary*/
  
  result=quad_calculate_value(x,quad_global_amount(),goptima,result);
  result=quad_calculate_value(x,quad_local_amount(),loptima,result);
  return result; 
}

double quad_safe_calculate(const double *x)
{
  double temp[DMAX];
  int i;
  
  for(i=0;i<g_dimension();i++)
    temp[i]=x[i];
  
  return quad_calculate(temp);
}

int quad_check_result(struct g_pop_data pdata)
{
  int i,j;
  int op_amount=0; /*Keeps track how many global optima are found in population (may be same)*/
  int result=0; /*The final result*/
  double *tmp; /*Stores the global optima among pdata.population*/
  
  /*Memory for tmp*/
  if((tmp=malloc(pdata.pop_size*g_dimension()*sizeof(double)))==NULL){
    fprintf(stderr,"Error: Could not allocate memory\n");
    exit(-1);
  }		
  
  /*First we check that the data is inside bounds and discard elements which do not 
    define any global optima (speeds up the rest of the calculations)*/
  for(i=0;i<pdata.pop_size;i++){    
    /*We check that all values are within bounds, which should be the case. If not, either 
      the population array is broken or our actual population vectors are broken. Either way 
      there is no point continuing, so error message is displayed and exit is called.*/    
    for(j=0;j<g_dimension();j++)
      if(pdata.population[i*pdata.max_Dsize+j]<llimit[j] || pdata.population[i*pdata.max_Dsize+j]>ulimit[j]){
	fprintf(stderr,"Error: Broken population array, values not within bounds\n");
	exit(-1);		
      }
    
    /*Is the current member close enough to the global optimum value?*/
    if(quad_calculate(&pdata.population[i*pdata.max_Dsize])<=goptima[0].optimum_value+pdata.accuracy){
      for(j=0;j<g_dimension();j++)
    	tmp[op_amount*g_dimension()+j]=pdata.population[i*pdata.max_Dsize+j];
      op_amount++;
    }		
  }
  
  
  /*Now we check which of the optima are not duplicates*/
  for(i=0;i<quad_global_amount();i++){ /*Loop through all the real optima*/
    for(j=0;j<op_amount;j++){ /*Loop through the found optima in tmp*/			
      if(g_euclidean(quad_optima_location(i),&tmp[j*g_dimension()],g_dimension())<(mindist/2-SMALL)){ 
    	result++;
    	break;
      }
    }
  }  
  free(tmp);      	
  return result;
}

int quad_check_lresult(struct g_pop_data pdata)
{
  int i,j;
  int op_amount=0; /*Keeps track how many local optima are found in population (may be same)*/
  int result=0; /*The final result*/
  double *tmp; /*Stores the local optima among pdata.population*/
  double tmp2=0; /*Just a temporary helper variable*/
  
  if(g_local_amount()==0){
    fprintf(stderr,"Error: called g_check_lresult for function that does not contain local optima\n");
    exit(-1);
  } 

  /*Memory for tmp*/
  if((tmp=malloc(pdata.pop_size*g_dimension()*sizeof(double)))==NULL){
    fprintf(stderr,"Error: Could not allocate memory\n");
    exit(-1);
  }		

  /*First we check that the data is inside bounds and discard elements which do not 
    define any local optima (speeds up the rest of the calculations)*/
  for(i=0;i<pdata.pop_size;i++){
    /*We check that all values are within bounds, which should be the case. If not, either 
      the population array is broken or our actual population vectors are broken. Either way 
      there is no point continuing, so error message is displayed and exit is called.*/    
    for(j=0;j<g_dimension();j++)
      if(pdata.population[i*pdata.max_Dsize+j]<llimit[j] || pdata.population[i*pdata.max_Dsize+j]>ulimit[j]){
	fprintf(stderr,"Error: Broken population array, values not within bounds\n");
	exit(-1);		
      }
    
    tmp2=quad_calculate(&pdata.population[i*pdata.max_Dsize]);
    /*Is the current member close enough to the upper limit for local optima and no smaller than the 
      lower limit (not global)?*/
    if(tmp2<=local_ob[1]+pdata.accuracy && tmp2>=local_ob[0]){
      for(j=0;j<g_dimension();j++)
	tmp[op_amount*g_dimension()+j]=pdata.population[i*pdata.max_Dsize+j];
      op_amount++;
    }		
  }

  /*Now we check which of the candidates really are local optima and remove dublicates*/
  for(i=0;i<quad_local_amount();i++){ /*Loop through all the real optima*/
    for(j=0;j<op_amount;j++){ /*Loop through the found optima in tmp*/			
      if(g_euclidean(quad_loptima_location(i),&tmp[j*g_dimension()],g_dimension())<(minldist/2-SMALL) &&	 
	 quad_calculate(&tmp[j*g_dimension()])<=loptima[i].optimum_value+pdata.accuracy){ 
	result++;
	break;
      }
    }
  }  
  free(tmp);      	
  return result;
}

void quad_get_initialization_point(double *x)
{
  int i=0;
  for(i=0;i<g_dimension();i++)
    x[i]=g_randu()*(ulimit[i]-llimit[i])+llimit[i];
}

const double* quad_constraint_function(int number)
{
  return g_get_constraint_function(number);
}

int quad_constraint_amount(void)
{
  return 2*g_dimension();
}

const double* quad_lower_constraints(void)
{
  return llimit;
}

const double* quad_upper_constraints(void)
{
  return ulimit;
}

double quad_optima_value(void)
{
  return MINIMA;
}
