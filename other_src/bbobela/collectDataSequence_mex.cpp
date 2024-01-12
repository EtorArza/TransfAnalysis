#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <math.h>
#include <algorithm>

#ifdef USE_OpenMP
#include <omp.h>
#endif

using namespace std;

// X: row-wise matrix
// This func doesn't allocate mem for outputs: sequence, deltaX
void calSequenceDeltaX(const double* X, const int nrows, const int ncols, double* sequence, double* deltaX) { 
	bool* mask = new bool[nrows]();
	
	sequence[0] = 1;
	mask[0] = true;
	
	double* dis = new double[nrows];
	for(int i=1; i < nrows; i++) {

		int this_row = sequence[i-1] - 1;

#ifdef USE_OpenMP
		#pragma omp parallel for
#endif	
		for(int j=0; j < nrows; j++) {
		
			if(!mask[j]) {
				dis[j] = 0;
				for(int c = 0; c < ncols; c++) {
					double v = X[this_row*ncols+c]-X[j*ncols+c];
					dis[j] += v*v;
				}
			}
		}	

		//find nearest neighbor
		double min_dist = numeric_limits<float>::max();
		int min_ind = -1;
		for(int j=0; j < nrows; j++) {
			if(!mask[j]) {
				if(min_dist > dis[j]) {
					min_dist = dis[j];
					min_ind = j;
				}
			}
		}

		sequence[i] = min_ind + 1;
		deltaX[i-1] = sqrt(min_dist);
		mask[min_ind] = true;
	}

	delete []dis;
	delete []mask;
}

#ifndef BUILD_TEST		//mex function

#include <matrix.h>
#include <mex.h>

//#define PRINT_DEBUG

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  	//[sequence, deltaX] = metafeature(double** X) 

  	mexPrintf("collectDataSequence_mex!\n");

  	size_t nrows, ncols;
  	double* X;    // row wise
  
  	if(nrhs != 1) {
    	mexPrintf("Incorrect input arguments\n");
    	mexPrintf("Usage: [sequence, deltaX] = metafeature(double** X). X col-wise matrix\n");
    	return;
  	}

  	const mwSize *dims = mxGetDimensions(prhs[0]);
  	nrows = dims[1]; ncols = dims[0];
#ifdef PRINT_DEBUG
  	mexPrintf("rows: %d, cols: %d\n", nrows, ncols);
#endif

  	X = mxGetPr(prhs[0]);
  	
  	plhs[0] = mxCreateDoubleMatrix(1, nrows, mxREAL); 
  	double *sequence = mxGetPr(plhs[0]);

  	plhs[1] = mxCreateDoubleMatrix(1, nrows-1, mxREAL); 
  	double *deltaX = mxGetPr(plhs[1]);

  	calSequenceDeltaX(X, nrows, ncols, sequence, deltaX);
  	mexPrintf("Done!\n");
}

#else

#include <sys/time.h>

unsigned int getTime() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

double* readDataFromFile(const char* filename, int &nrows, int &ncols) {
	ifstream infile(filename);
    if(!infile) {
        cout << "Error: cannot open file " << filename << " to read" << endl;
        return NULL;
    }

    string line;
    nrows = ncols = 0;

    // find nrows, ncols
    while (getline(infile, line)) {

		if (line.length() == 0)
			continue;

		if (nrows == 0) { //first row
			stringstream  ss(line);
            string  token;

            while (ss >> token)
            	ncols++;
		}

		nrows++;
	}

	cout << "nrows: " << nrows << " ncols: " << ncols << endl;

	infile.clear();
	infile.seekg(0);

	double* X = new double[nrows*ncols];
	if(X == NULL) {
		cout << "Not enough mem" << endl;
		return NULL;
	}

	// now read data
	int row = 0;
    while (getline(infile, line)) {

		if (line.length() == 0)
			continue;

		stringstream  ss(line);
        string  token;

        int col = 0;
        while (ss >> token) {
        	X[row*ncols + col] = atof(token.c_str());
        	col++;
        }
        row++;
	}

    return X;
}

void printResult(const double* X, const double* sequence, const double* deltaX, int printnrows, int ncols) {
	cout << "X" << endl;
	for(int i=0; i < printnrows; i++) {
		for(int j=0; j < ncols; j++)
			cout << X[i*ncols + j] << " ";
		cout << endl;
	}
	cout << "..." << endl;

	cout << "Sequence" << endl;
	for(int i=0; i < printnrows; i++)
		cout << sequence[i] << " ";
	cout << endl;

	cout << "DetalX" << endl;
	for(int i=0; i < printnrows; i++)
		cout << deltaX[i] << " ";
	cout << endl;
}

int main(int argc, const char* argv[]) {

	double* X;
	int nrows, ncols;

	X = readDataFromFile((const char*)"X_D10_C1000.dat", nrows, ncols);

	unsigned int start_t = getTime();
	double* sequence = new int[nrows];
	double* deltaX = new double[nrows - 1];
	calSequenceDeltaX(X, nrows, ncols, sequence, deltaX);
	cout << "Procesing time: " << getTime() - start_t << endl;

	printResult(X, sequence, deltaX, 10, ncols);

	delete [] sequence;
	delete [] deltaX;

	return 0;
}

#endif