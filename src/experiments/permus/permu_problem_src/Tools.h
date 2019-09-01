#ifndef TOOLS_H
#define TOOLS_H
/*
 *  Tools.h
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 11/21/11.
 *  Copyright 2011 University of the Basque Country. All rights reserved.
 *
 */
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <cmath>

using std::istream;
using std::ostream;
using namespace std;
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::stringstream;

/*
 * Returs the first position at which value appears in array. If it does not appear, then it returns -1;
 */
int Find(int *array, int size, int value);

/*
 * Calculates Kullback-Leibeler divergence between P and Q distributions.
 */
double KullbackLeibelerDivergence(double *P, double *Q, int size);

/*
 * Calculates Total Variation divergence between P and Q distributions.
 */
double TotalVariationDivergence(double *P, double *Q, int size);

/*
 * It determines if the given int sequecen if it is indeed a permutation or not.
 */
bool isPermutation(int *permutation, int size);

/*
 * Generates a random permutation of size 'n' in the given array.
 */
void GenerateRandomPermutation(int *permutation, int n);

/*
 * Determines if a given string contains a certain substring.
 */
bool strContains(const string inputStr, const string searchStr);

/*
 * Prints in standard output 'length' integer elements of a given array.
 */
void PrintArray(int *array, int length, string text);

/*
 * Prints in standard output 'length' long double elements of a given array.
 */
void PrintArray(long double *array, int length, string text);

/*
 * Prints the given doubles array in the standard output.
 */
void PrintArray(double *array, int length, string text);


/*
 * Applies the random keys sorting strategy to the vector of doubles
 */
void RandomKeys(int *a, double *criteriaValues, int size);

/*
 * Calculates the tau Kendall distance between 2 permutations.
 */
int Kendall(int *permutationA, int *permutationB, int size);

/*
 * Calculates the Kendall tau distance between 2 permutations.
 */
int Kendall(int *permutationA, int *permutationB, int size, int *m_aux);

/*
 * Calculates the Kendall tau distance between 2 permutations.
 * Auxiliary parameters are used for multiple executions.
 */
int Kendall(int *permutationA, int *permutationB, int size, int *m_aux, int *invertedB, int *composition, int *v);

/*
 * Calculates the Cayley distance between 2 permutations.
 */
int Cayley(int *permutationA, int *permutationB, int size);

/*
 * Calculates the Cayley distance between 2 permutations.
 */
int Cayley(int *permutationA, int *permutationB, int size, int *invertedB, int *composition, int *elemsToCycles, int *maxPosInCycle, int *freeCycle);
int FindNewCycle(int *freeCycle, int size);
int NextUnasignedElem(int *elemsToCycles, int size);
int CalculateDistance(int *sigma, int size);

/*
 * Calculates the length of the longest increasing subsequence in the given array of ints.
 */
int getLISLength(int *sigma, int size);

/*
 * Implements the compose of 2 permutations of size n.
 */
void Compose(int *s1, int *s2, int *res, int n);

/*
* Calculates V_j-s vector.
*/
void vVector(int *v, int *permutation, int n);

/*
 *  Optimized version by Leti of the V_j-s vector calculation.
 */
void vVector_Fast(int *v, int *permutation, int n, int *m_aux);

/*
 * Inverts a permutation.
 */
void Invert(int *permu, int n, int *inverted);

/*
 * This method moves the value in position i to the position j.
 */
void InsertAt(int *array, int i, int j, int n);

/*
 * Calculates the factorial of a solution.
 */
long double factorial(int val);

/*
 * This method applies a swap of the given i,j positions in the array.
 */
void Swap(int *array, int i, int j);

// not copied the ones below this


/*
 * Calculate the Hamming distance between two permutations
 */

int Hamming_distance(int* sigma1, int* sigma2, int len);

/*
 * Set timer to 0.
 */
void tic();

/*
 * Return time since last tic().
 */
float toc();

/*
 * Convert to string.
 */
template <class T>
string toString(const T &t, bool *ok = NULL)
{
    ostringstream stream;
    stream << t;
    if (ok != NULL)
        *ok = (stream.fail() == false);
    return stream.str();
}

/*
 * Convert to string.
 * https://stackoverflow.com/questions/3909272/sorting-two-corresponding-arrays
 * sort 2 arrays simultaneously. The values used as keys are placed in A. If reverse == false, the order is ascending.
 */
template <class A, class B>
void QuickSort2Desc(A a[], B b[], int l, int r, bool ascending=false)
{
    int i = l;
    int j = r;

    int coef = 1;
    if (ascending==true)
    {
        coef = -1;
    }

    A v = a[(l + r) / 2];

    
    do
    {
        while (coef*a[i] > coef*v)
            i++;
        while (coef*v > coef*a[j])
            j--;
        if (i <= j)
        {
            std::swap(a[i], a[j]);
            std::swap(b[i], b[j]);
            i++;
            j--;
        };
    } while (i <= j);
    if (l < j)
        QuickSort2Desc(a, b, l, j, ascending);
    if (i < r)
        QuickSort2Desc(a, b, i, r, ascending);
}

/*
 * Return wether two vectors are equal or not.
 */
template <class T>
bool compare_vectors(T *vec1, T *vec2, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (vec1[i] != vec2[i])
        {
            return false;
        }
    }
    return true;
}

/*
Function to find all the repeated rows on a matrix
Writes in  bool *is_ith_position_repeated (true --> vector is a repetition, false--> vector is not a repetition)
*/
template <class T>
void which_indexes_correspond_to_repeated_vectors(T **vec_array, int vec_len, int n_of_vecs, bool *is_ith_position_repeated, bool is_known_last_repeated_indexes)
{
    if (n_of_vecs == 1)
    {
        is_ith_position_repeated[0] = false;
        return;
    }
    else if (n_of_vecs == 2)
    {
        is_ith_position_repeated[0] = false;
        is_ith_position_repeated[1] = compare_vectors(vec_array[0], vec_array[1], vec_len);
        return;
    }

    is_ith_position_repeated[0] = false;
    is_ith_position_repeated[1] = compare_vectors(vec_array[0], vec_array[1], vec_len);

    for (int i = 2; i < n_of_vecs; i++)
    {
        if (is_known_last_repeated_indexes && not is_ith_position_repeated[i])
        {
            continue;
        }
        for (int j = i - 1; j >= 0; j--)
        {
            is_ith_position_repeated[i] = false;
            if (compare_vectors(vec_array[i], vec_array[j], vec_len))
            {
                is_ith_position_repeated[i] = true;
                break;
            }
        }
    }
}

/*
Shuffle vector given its length.
*/
void shuffle_vector(int *vec, int len);


/*
Get random integer on interval [0, max - 1], faster but slightly biased
*/
int random_integer_fast(int max);

/*
Get random integer on interval [min, max - 1], faster but slightly biased
*/
int random_integer_fast(int min, int max);

// Get random integer on interval [min, max - 1]
// https://ericlippert.com/2013/12/16/how-much-bias-is-introduced-by-the-remainder-technique/
int random_integer_uniform(int min, int max = 0);

// chooses a random integer from {0,1,2, range_max - 1}, at uniform (may be a little slower)
int random_range_integer_uniform(int range_max);

// return a random uniform float on the interval [0,1]
float random_0_1_float();

// apply sigmoid function
float sigmoid(float x);

// Choose an index given the probabilities
int choose_index_given_probabilities(float *probabilities_array, int max_index);


// Choose an index given positive weights
int choose_index_given_weights(float *weights_array, int max_index);

// Sample from a bernouilli distribution.
bool coin_toss(float p_of_true);

// round a float into the nearest integer
int tools_round(float x);

// compute the average value of the elements on the array
template <class T>
float Average(T *array, int len)
{

    float sum = 0;

    for (int i = 0; i < len; i++)
    {
        sum += array[i];
    }

    return (float)sum / len;
}

// compute the variance of the elements on the array
template <class T>
float Variance(T *array, int len)
{

    float mean = Average(array, len);

    float var = 0;
    for (int i = 0; i < len; i++)
    {
        var += (array[i] - mean) * (array[i] - mean);
    }

    return (float)var / len;
}

// Normalize a vector so that the sum of all the elements on it is 1
template <class T>
void normalize_vector(T *array, int len)
{
    int sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += array[i];
    }
    for (int i = 0; i < len; i++)
    {
        array[i] = array[i] / sum;
    }
}




template <class T>
void PrintMatrix(T **M, int m, int n)
{

	cout << "\n";
	for (int i = 0; i < m; i++)
	{
		cout << "| i = " << i << " ( ";
		for (int j = 0; j < n; j++)
		{
			cout << M[i][j] << " ";
		}
		cout << ")\n";
	}
}

// Sum the elements from pos i to pos j-1. The python equivalent of sum(v[i:j])
template <class T>
T sum_slice_vec(T *v, int i, int j)
{
    T res = 0;
    for (int k = i; k < j; k++)
    {
        res += v[k];
    }
    return res;
}


// Sum the absolute value of real valued elements from pos i to pos j-1. The python equivalent of sum(v[i:j])
template <class T>
T sum_abs_val_slice_vec(T *v, int i, int j)
{
    T res = 0;
    for (int k = i; k < j; k++)
    {
        res += abs(v[k]);
    }
    return res;
}

template <class T>
int argmax(T *v, int len){
    T max = v[0];
    int res = 0;
    for (int i = 1; i < len; i++)
    {
        if (v[i] > max)
        {
            max = v[i];
            res = i;
        }
    }
    return res;
}


template <class T>
int argmin(T *v, int len){
    T max = v[0];
    int res = 0;
    for (int i = 1; i < len; i++)
    {
        if (v[i] < max)
        {
            max = v[i];
            res = i;
        }
    }
    return res;
}


template <class T>
void copy_vector(T *v_res, T*v_ref, int len){
	memcpy(v_res, v_ref, sizeof(T)*len);
}


inline
float fast_exp(float x) {
  x = 1.0 + x / 256.0;
  x *= x; x *= x; x *= x; x *= x;
  x *= x; x *= x; x *= x; x *= x;
  return x;
}



class PermuTools
{
public:

	PermuTools(int n);
	~PermuTools();

    int n;
    int* random_permu1;
    int* random_permu2;
    int* temp_array;
    float* temp_array_float;
    int* identity_permu;
    float** first_marginal;

    void combine_permus(int** permu_list, float* coef_list, int* res);
    void compute_first_marginal(int** permu_list, int m);
    float get_distance_to_marginal(int* permu);
    int choose_permu_index_to_move(float* coef_list);

private:
    void convert_to_permu(int* res);
};


class stopwatch
{
public:
    stopwatch(){tt_tic = getTick();}
    ~stopwatch(){}
    void tic() { tt_tic = getTick();}
    float toc() { return (float) (getTick() - tt_tic);}
private:
    double getTick();
    double tt_tic;

};




#endif /* TOOLS_H */
