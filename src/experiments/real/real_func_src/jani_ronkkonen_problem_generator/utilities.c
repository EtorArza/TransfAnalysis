/*
  Multimodal function generator                      
  
  Implementations for the utility functions. 
  
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

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "utilities.h"
#include "generator.h"
#include "gen_private.h"

#define BMAX 10 /*The maximum degree of bezier curves*/

static int rn=0;/*The seed for g_randu*/
static double P[DMAX][BMAX+1]; /*The control points for Bezier curves*/
static int bezier_dimensions[DMAX];  /*The degree of Bezier curves*/
static double A[DMAX][DMAX]; /*The rotation matrix*/
static double AT[DMAX][DMAX]; /*The transpose of the rotation matrix A*/
static double constr[2*DMAX][DMAX+1]; /*The multipliers for constraint functions*/

double g_randu(void)
{
  const int a = 16807, m = 2147483647; /*Park&Miller*/
  const int q = 127773, r = 2863; /*Schrage*/
  const double idouble = 1./((double)m);
  int k;
  
  k = rn/q;
  rn = a*(rn-k*q)-r*k;
  if (rn<0) rn+=m;
  return idouble*rn;
}

void g_init_seed(int seed)
{
  if(seed==-1){
    rn=(int)time(0) ^ 666666666;	
  }else{
    /*xor with mask to avoid clashes with s=0*/
    rn = seed ^ 666666666;
  }
  if (rn<=0) rn=-rn+1;
  
  g_randu();/*discard the first three numbers of the sequence*/
  g_randu();
  g_randu();
}

double g_randn(void)
{
  static int state=0; /*Keeps track do we return first or second*/
  static double g2; /*Second gaussian value returned each other round*/
  double g1; /*Value returned first*/
  double u1, u2, s; /*Helper variables*/
  
  if(state==0){
    do{
      u1=2.0*g_randu()-1.0;
      u2=2.0*g_randu()-1.0;
      s=u1*u1+u2*u2; 			
    }while(s>=1.0 || s==0.0);
    state=1;
    g1=u1*sqrt(-2.0*log(s)/s);
    g2=u2*sqrt(-2.0*log(s)/s);
    return(g1);
  }else{
    state=0;
    return(g2);
  }  
}

double g_euclidean(const double *a, const double *b, int D)
{
  double result=0;
  int i;
  
  for(i=0;i<D;i++)
    result=result+(a[i]-b[i])*(a[i]-b[i]);
  result=sqrt(result);
  return result;
}

FILE *g_file_open(char *name, char *method)
{
  FILE* opened_file=NULL;
  
  opened_file = fopen(name,method);
  if(opened_file == NULL){
    fprintf(stderr,"Error: Cannot open file %s\n",name);
    exit(-1);
  }
  return opened_file;
}

void g_constraints(double *x, int D, const double *llimit, const double *ulimit)
{
  int i;

  for(i=0;i<D;i++)
    while(x[i]<llimit[i] || x[i]>ulimit[i]){
      if (x[i]< llimit[i]) 
	x[i] = 2.0*llimit[i] - x[i];
      if (x[i]>ulimit[i]) 
	x[i] = 2.0*ulimit[i] - x[i];
    }
}

int g_factorial(int n)
{
  int facn=1,i=0;
  
  for(i=2;i<=n;i++)
    facn=facn*i;
  return facn;
}

void g_check_pointer(const void *ptr)
{
  if (ptr==NULL){
    fprintf(stderr,"Error: given pointer is empty \n");
    exit(-1);
  }
}

void g_bezier_shift_(double *coord)
{
  int i,j;
  long facn=0;
  long b[BMAX+1]; /*stores binomical coefficients*/
  double shifted=0.0;
  
  for(i=0;i<g_dimension();i++)
    {
      facn=g_factorial(bezier_dimensions[i]); /*The factorial of bezier degree*/				
      for(j=0;j<=bezier_dimensions[i];j++) /*Binomical coefficients*/
	b[j]=facn/(g_factorial(j)*(g_factorial(bezier_dimensions[i]-j)));		       
      
      shifted=0.0;
      for(j=0;j<=bezier_dimensions[i];j++){ /*The shifted point*/
	shifted=shifted+(double)b[j]*P[i][j]*
	  pow(1-coord[i],bezier_dimensions[i]-j)*
	  pow(coord[i],j);
      }
      coord[i]=shifted; /*Replace the old coord value with the modified value*/		
    }
}

void g_bezier_shift(double *coord, const double *llimit, const double *ulimit) {
  int i, j;
  long facn = 0;
  long b[BMAX + 1];		/*stores binomical coefficients */
  double shifted = 0.0;

  for (i = 0; i < g_dimension (); i++) {
      facn = g_factorial (bezier_dimensions[i]);	/*The factorial of bezier degree */
      for (j = 0; j <= bezier_dimensions[i]; j++)	/*Binomical coefficients */
      	b[j] = facn / (g_factorial (j) * (g_factorial (bezier_dimensions[i] - j)));

      shifted = 0.0;
      /*Normalize coordinates to interval [0, 1]*/
      if (llimit[i] != 0.0 || ulimit[i] != 1.0) {
            coord[i] = coord[i] - llimit[i];
            coord[i] = coord[i] / (ulimit[i] - llimit[i]);
      }
      for (j = 0; j <= bezier_dimensions[i]; j++) {			/*The shifted point */
	  shifted = shifted + (double) b[j] * P[i][j] * pow (1 - coord[i], bezier_dimensions[i] - j) * pow (coord[i], j);
	}
      coord[i] = shifted;	/*Replace the old coord value with the modified value */
      /*De-normalize the coordinates*/
      if (llimit[i] != 0.0 || ulimit[i] != 1.0) {
            coord[i] = coord[i] * (ulimit[i] - llimit[i]);
            coord[i] = coord[i] + llimit[i];
      }
  }
}

void g_inverse_bezier_shift_(double *coord, const double *llimit, const double *ulimit)
{
  long i,j;
  long facn=0;
  long b[BMAX+1]; /*stores binomical coefficients*/
  double shifted=0.0,coord_orig,upper,lower;
  
  for(i=0;i<g_dimension();i++)
  {
      if(coord[i]!=1 && coord[i]!=0)
	{
	  facn=g_factorial(bezier_dimensions[i]); /*The factorial of bezier degree*/ 	
	  for(j=0;j<=bezier_dimensions[i];j++) /*Binomical coefficients*/
	    b[j]=facn/(g_factorial(j)*(g_factorial(bezier_dimensions[i]-j)));
	  
	  coord_orig=coord[i]; /*The value to reach is stored to coord_orig*/		
	  upper=ulimit[i];
	  lower=llimit[i];
	  coord[i]=lower+(upper-lower)/2; /*We start from the middle*/
	  do{
	    shifted=0.0;
	    for(j=0;j<=bezier_dimensions[i];j++){ /*The shifted point*/	
	      shifted=shifted+(double)b[j]*P[i][j]*
		  pow(1-coord[i],bezier_dimensions[i]-j)*
		  pow(coord[i],j);
	    }
	    /*Depending on which half the point was, the search area is halved each round*/
	    if(shifted>coord_orig){ /*Value too big*/
	      upper=coord[i];
	      coord[i]=lower+(upper-lower)/2;
	    }else if(shifted<coord_orig){ /*Value too small*/
	      lower=coord[i];
	      coord[i]=lower+(upper-lower)/2;
	    }			    			
	  }while((coord_orig-SMALL)>shifted || shifted>(coord_orig+SMALL));				
	}	       
  }
}

void g_inverse_bezier_shift(double *coord, const double *llimit, const double *ulimit) {
      long i, j;
      long facn = 0;
      long b[BMAX + 1];		/*stores binomical coefficients */
      double shifted = 0.0, coord_orig, upper, lower;

      for (i = 0; i < g_dimension (); i++)
      {
            /*Normalize coordinates to interval [0, 1]*/
            if (llimit[i] != 0.0 || ulimit[i] != 1.0) {
                  coord[i] = coord[i] - llimit[i];
                  coord[i] = coord[i] / (ulimit[i] - llimit[i]);
            }

            if (coord[i] != 1 && coord[i] != 0) {
                  facn = g_factorial (bezier_dimensions[i]);	/*The factorial of bezier degree */
                  for (j = 0; j <= bezier_dimensions[i]; j++) {	/*Binomical coefficients */
                        b[j] = facn / (g_factorial (j) * (g_factorial (bezier_dimensions[i] - j)));
                  }
                  coord_orig = coord[i];	/*The value to reach is stored to coord_orig */
                  upper = 1;
                  lower = 0;
                  coord[i] = lower + (upper - lower) / 2;	/*We start from the middle */
                  do {
                        shifted = 0.0;
                        for (j = 0; j <= bezier_dimensions[i]; j++) {		/*The shifted point */
                              shifted = shifted + (double) b[j] * P[i][j] * pow (1 - coord[i], bezier_dimensions[i] - j) * pow (coord[i], j);
                        }
                        /*Depending on which half the point was, the search area is halved each round */
                        if (shifted > coord_orig) {		/*Value too big */
                              upper = coord[i];
                              coord[i] = lower + (upper - lower) / 2;
                        }
                        else if (shifted < coord_orig) {		/*Value too small */
                              lower = coord[i];
                              coord[i] = lower + (upper - lower) / 2;
                        }
                  }
                  while ((coord_orig - SMALL) > shifted || shifted > (coord_orig + SMALL));

            }
            /*De-normalize the coordinates*/
            if (llimit[i] != 0.0 || ulimit[i] != 1.0) {
                  coord[i] = coord[i] * (ulimit[i] - llimit[i]);
                  coord[i] = coord[i] + llimit[i];
            }
      }
}

void g_bezier_initialize(FILE *init_file)
{
  int i,j;
  char init_data[DMAX][(BMAX+1)*10]; /*All data from initialization file is first read here*/	
  char *split = NULL; /*Helper pointer for strtok*/
  double value=0.0;

  for (i=0;i<g_dimension();i++){
    if(fgets(init_data[i],BMAX*10,init_file)==NULL){
      fprintf(stderr,"Error: Too few parameters in initialization file\n");
      exit(-1);
    }	                  
  }
  
  /*The Bezier control points*/	
  for (i=0;i<g_dimension();i++) /*Loop for lines*/
    {
      split = (char*)strtok(init_data[i], " ");
      if (atof(split)==-1){ /*The default value*/
	bezier_dimensions[i]=1; /*there is always 1 more control point than bezier degree*/
	P[i][0]=0.0;
	P[i][1]=1.0;
	if(g_output_level()>1)	     
	  printf("Using default value (linear) for Bezier line %d\n",i+1);
	
      }else if(atof(split)<-1){ /*Randomly generated points*/	
	if(atoi(split)<-BMAX){
	  if(g_output_level()>1)	     
	    printf("Too long Bezier line %d, using degree %d\n",i+1,BMAX);
	  bezier_dimensions[i]=BMAX;
	}else{
	  bezier_dimensions[i]=abs(atoi(split));
	}
	P[i][0]=0.0;			
	for(j=1;j<=bezier_dimensions[i];j++)
	  {
	    value=g_randu();
	    if(value==0.0) /*Just a precaution to make sure the values are increasing*/
	      value=value+SMALL;
	    P[i][j]=P[i][j-1]+value;	
	  }
	for(j=1;j<=bezier_dimensions[i];j++) /*Scaling*/
	  P[i][j]=P[i][j]/P[i][bezier_dimensions[i]];	
	
      }else if((atof(split)==0.0)){ /*No specials, just read the values*/
	j=0;
	P[i][j]=0.0;
	split = (char*)strtok(NULL, " ");			
	while(split!=NULL && j<BMAX){		
	  if (atof(split)<0.0 || atof(split)>1.0 || atof(split)<=P[i][j]){
	    fprintf(stderr,"Error: Unsuitable parameter %d for Bezier line %d\n",j+1,i+1);
	    exit(-1);
	  }
	  j++;
	  P[i][j]=atof(split);				
	  split = (char*)strtok(NULL, " ");
	}		
	if (P[i][j]!=1.0){ /*Last value must be 1*/
	  fprintf(stderr,"Error: Last value in Bezier line %d was not 1",i+1); 
	  fprintf(stderr," or wrong amount of values (maximum degree %d)\n",BMAX);
	  exit(-1);
	}
	bezier_dimensions[i]=j;
	
      }else{ /*The first value was not 0 or negative*/
	fprintf(stderr,"Error: Unsuitable first value for Bezier line %d\n",i+1);
	exit(-1);
      }
    }
}

void g_bezier_data(void)
{
  int i,j;
  for(i=0;i<g_dimension();i++){
    printf("B%d=[%g",i+1,P[i][0]);
    for(j=1;j<=bezier_dimensions[i];j++)
      printf(",%g",P[i][j]);
    printf("]\n");
  }
}

void g_init_transform(void)
{
  double scalar=0, line[DMAX], norm=0;
  int i,j,k;
  
  for (i=0;i<g_dimension();i++){		
    /*random values for matrix A:s i:th row, first step*/
    for (j=0;j<g_dimension();j++)
      A[i][j]=g_randn();		 
    
    /*second step*/
    /*if j<=i, the line must be 0*/
    for(k=0;k<g_dimension();k++)		
      line[k]=0.0;
    
    for(j=0;j<i;j++){
      scalar=0;/*scalar product*/
      for(k=0;k<g_dimension();k++)				
	scalar=scalar+A[i][k]*A[j][k];			
      for(k=0;k<g_dimension();k++)			
	line[k]=line[k]+scalar*A[j][k];			
    }
    for(k=0;k<g_dimension();k++)		
      A[i][k]=A[i][k]-line[k];
    
    /*third step*/
    norm=0;
    for(k=0;k<g_dimension();k++)			
      norm=norm+A[i][k]*A[i][k];
    
    norm=sqrt(norm);
    for(k=0;k<g_dimension();k++)		
      A[i][k]=A[i][k]/norm;					
  }
  /*We generate the transpose of A*/
  for(i=0;i<g_dimension();i++) 
    for(j=0;j<g_dimension();j++)
      AT[i][j]=A[j][i]; 
}

void g_unrotate(double *x)
{
  int i,j;
  double tmp[DMAX];
  
  for (i=0;i<g_dimension();i++){ /*Unrotation*/			
    tmp[i]=0;	
    for(j=0;j<g_dimension();j++)			
      tmp[i]=tmp[i]+AT[i][j]*x[j];					
  }
  for (i=0;i<g_dimension();i++)
    x[i]=tmp[i];
}

void g_rotate(double *x)
{
  int i,j;
  double tmp[DMAX];
  
  for (i=0;i<g_dimension();i++){			
    tmp[i]=0;	
    for(j=0;j<g_dimension();j++)			
      tmp[i]=tmp[i]+A[i][j]*x[j];					
  }
  for (i=0;i<g_dimension();i++)
    x[i]=tmp[i];
}

void g_rotate_multipliers(double *x)
{
  int i,j;
  double tmp[DMAX];
  
  for (i=0;i<g_dimension();i++){			
    tmp[i]=0;	
    for(j=0;j<g_dimension();j++)			
      tmp[i]=tmp[i]+AT[j][i]*x[j];					
  }
  for (i=0;i<g_dimension();i++)
    x[i]=tmp[i];
}

void g_matrix_multiply(double *F, double *S, double *R)
{
  int i,j,k;
  double tmp=0,tmp2[DMAX][DMAX];

  for (i=0;i<g_dimension();i++){
    for (j=0;j<g_dimension();j++){
      tmp=0; 
      for (k=0;k<g_dimension();k++){
	tmp=tmp+F[i*DMAX+k]*S[k*DMAX+j];/*F[i][k]*S[k][j]*/
      }
      /*Temporary storage in case R and F or S are pointers to same array*/
      tmp2[i][j]=tmp; 
    }
  }
  for (i=0;i<g_dimension();i++){
    for (j=0;j<g_dimension();j++){
      R[i*DMAX+j]=tmp2[i][j];/*R[i][j]*/
    }
  }
}

void g_cov_rotate(double *target)
{
  g_matrix_multiply(A[0],target,target);
  g_matrix_multiply(target,AT[0],target);
}

void g_matrix_inversion(double *orig, double *inv) 
{
  int i,j;
  int column=0, row=0;
  double multiplier=0.0;
  double otmp[DMAX][DMAX],invtmp[DMAX][DMAX]; 

  for (i=0;i<g_dimension();i++) { 
    for (j=0;j<g_dimension();j++) {/*invtmp is set to be identitymatrix*/
      if(i==j)
	invtmp[i][j]=1;
      else
	invtmp[i][j]=0;
    }
    for (j=0;j<g_dimension();j++)/*Copy of orig to otmp*/
      otmp[i][j]=orig[i*DMAX+j];/*orig[i][j]*/      
  }

  for (column=0;column<g_dimension();column++) {
    row=column;
    while (row<g_dimension() && otmp[row][column]==0) 
      row++;    
    if (otmp[row][column]==0) {
      printf("Error: Inversion of matrix failed\n");
      exit(-1);
    }    
    for (i=0;i<g_dimension();i++) {
      if (i!=row) {
	multiplier=-otmp[i][column]/otmp[row][column];
	for (j=0;j<g_dimension();j++) {
	  otmp[i][j]=otmp[i][j]+otmp[row][j]*multiplier;
	  invtmp[i][j]=invtmp[i][j]+invtmp[row][j]*multiplier;
	}
      }
    }
  }
  for (row=0;row<g_dimension();row++) {
    for (column=0;column<g_dimension();column++) {
      multiplier=otmp[row][row];
      invtmp[row][column]=invtmp[row][column]/multiplier;
    }
  }

  for (i=0;i<g_dimension();i++) /*copy of invtmp to inv*/
    for (j=0;j<g_dimension();j++)
      inv[i*DMAX+j]=invtmp[i][j]; /*inv[i][j]*/
 
}

void g_init_constraint_functions(const double *llimit, const double *ulimit, int rotation)
{
  int i,j;

  /*The constant terms for lower constraints*/
  for(i=0;i<2*g_dimension();i=i+2) 
    constr[i][g_dimension()]=llimit[i/2];

 /*The constant terms for upper constraints*/
  for(i=1;i<2*g_dimension();i=i+2) 
    constr[i][g_dimension()]=-ulimit[(int)ceil(i/2)];

  /*Multiplier terms for dimensions in unrotated case*/
  for(i=0;i<2*g_dimension();i++) 
    for(j=0;j<g_dimension();j++) {
      if(j==((int)floor(i/2))){
	if(i%2==0)  /*Lower constraints*/
	  constr[i][j]=-1;
	else 
	  constr[i][j]=1; /*Upper constraints*/	
      }else{
	constr[i][j]=0;
      }
    }

  /*Rotated function*/
  if(rotation==1){
      for(i=0;i<2*g_dimension();i++)
	g_rotate_multipliers(constr[i]);
    }
}

const double* g_get_constraint_function(int number)
{
  if(number>2*g_dimension() || number<0){
    fprintf(stderr,"Error: g_get_constraint_functions called with inappropriate value: %d\n",number);
    exit(-1);   
  }else{
    return constr[number];
  }
}
