#include <iostream>
#include <fstream>
#include <vector>
#include <set>

#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>

using namespace std;

//#define PRINT_DEBUG

double runDTreeClassification(const cv::Mat data_32f, const vector<int> folds) {

	int k = 10;
	
	int N = data_32f.cols - 1;
   
    CvDTreeParams params = CvDTreeParams(
        N, // max depth
        5,// min sample count
        0, // regression accuracy: N/A here
        true, // compute surrogate split, no missing data
        5, // max number of categories (use sub-optimal algorithm for larger numbers)
        0, // the number of cross-validation folds
        false, // use 1SE rule => smaller tree
        false, // throw away the pruned tree branches
        NULL // the array of priors
    );

    cv::Mat var_type = cv::Mat(data_32f.cols, 1, CV_8U );
    for(int i=0; i < N; i++)
        var_type.at<unsigned char>(i, 0) = CV_VAR_NUMERICAL;
    var_type.at<unsigned char>(N, 0) = CV_VAR_CATEGORICAL;

    //cv::Mat data_32f;
    //data.convertTo(data_32f, CV_32F);

    cv::Mat testvalue(1, N, CV_32F);
    double err = 0;
    // run k times
    for(int i=0; i < k; i++) {
    	int n_test = std::count (folds.begin(), folds.end(), i);
        int n_train = folds.size() - n_test;
        cv::Mat test(n_test, data_32f.cols, CV_32F);
        cv::Mat train(n_train, data_32f.cols, CV_32F);
        int test_ind = 0, train_ind = 0;
        for(int r=0; r < data_32f.rows; r++) {
            if(folds[r] == i)
                data_32f.row(r).copyTo(test.row(test_ind++));
            else
                data_32f.row(r).copyTo(train.row(train_ind++));
        }

        CvDTree dtree;
        dtree.train(train(cv::Range(0, train.rows), cv::Range(0,N)), CV_ROW_SAMPLE,
                 train.col(N), cv::Mat(), cv::Mat(), var_type, cv::Mat(), params); 

        cv::Mat pred_mat(test.rows, 1, CV_32F);
        for (int testid = 0; testid < test.rows; testid++) {
        	for (int c=0; c < N; c++)
            	testvalue.at<float>(0, c) = test.at<float>(testid, c);
            pred_mat.at<float>(testid, 0) = (float)dtree.predict(testvalue)->value;
        }

        // cal err rate
        int err_count = 0;
        for(int i=0; i < pred_mat.rows; i++) {
        	if(pred_mat.at<float>(i, 0) != test.at<float>(i, N))
        		err_count++;
        }

        err += 1.0 * err_count / pred_mat.rows;
        
    }
    err /= k;

	return err;
}

#ifndef BUILD_TEST

#include <matrix.h>
#include <mex.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  	//avgloss = ldaLoss_mex(double** X, double* label, double* folds) 

  	mexPrintf("dtreeLoss_mex 10-fold\n");

  	size_t nrows, ncols;
  	double* mat_X;   	// col-wise matrix
  	double* mat_label;
    double* mat_folds;
  
  	if(nrhs != 3) {
    	mexPrintf("Incorrect input arguments\n");
    	mexPrintf("Usage: avgloss = dtreeLoss_mex(double** X, double* label, double* folds). X col-wise matrix\n");
    	return;
  	}

  	const mwSize *dims = mxGetDimensions(prhs[0]);
  	nrows = dims[1]; ncols = dims[0];
#ifdef PRINT_DEBUG
  	mexPrintf("rows: %d, cols: %d\n", nrows, ncols);
#endif

  	mat_X = mxGetPr(prhs[0]);
  	mat_label = mxGetPr(prhs[1]);
    mat_folds = mxGetPr(prhs[2]);

  	cv::Mat data(nrows, ncols + 1, CV_32F);
  	for(int r=0; r < nrows; r++) {
		for(int c=0; c < ncols; c++)
			data.at<float>(r, c) = mat_X[r*ncols + c];
			
		data.at<float>(r, ncols) = mat_label[r];
	}

    vector<int> folds;
    for(int i=0; i < nrows; i++)
        folds.push_back((int)mat_folds[i]);
   
	double avgerr = 0;
	avgerr = runDTreeClassification(data, folds);
  	plhs[0] = mxCreateDoubleScalar(avgerr);
  	
  	mexPrintf("Done!\n");
}


#else // BUILD_TEST

cv::Mat loadDataFromFile(const char* filename) {
    ifstream infile(filename);
    if(!infile) {
        cout << "Error: cannot open file " << filename << " to read" << endl;
        return cv::Mat(0, 0, CV_64F);
    }
  
    string line;

    int nrows, ncols;
    
    nrows = 0;

    //parse header and count dimensions
    while (getline(infile, line)) {

        if (line.length() == 0)
            continue;

        if(nrows == 0) {
            ncols = 0;
            stringstream  ss(line);
            string  token;
            while ( ss >> token) {
                ncols++;
            }
        }

        nrows++;
    }

    // read matrixes
    infile.clear();
    infile.seekg(0);

    cv::Mat data;
    data.create(nrows, ncols, CV_32F);
    
    int line_index = 0;
    while (getline(infile, line)) {

        if (line.length() == 0)
            continue;

        stringstream ss(line);
        string token;
        int ind = 0;
        while ( ss >> token ) {
            data.at<float>(line_index, ind) = atof(token.c_str());
            ind++;
        }
        line_index++;
    }

    return data;
}

static float myroundf(float x) {
   return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}

vector<int> kfolds(const int nrows, const int k) {
    vector<int> folds;
    folds.clear();
    if (k == 1) {
        for(int i=0; i < nrows; i++)
            folds.push_back(1);
    }else {
        float i = (float)nrows / k;
        if(i < 1) {
            cout << "insufficient records: " << nrows << " with k= " << k << endl;
            return folds;
        }

        vector<int> vec_i;
        vec_i.push_back(0);
        for(int ind=1; ind < k; ind++)
            vec_i.push_back((int)myroundf(i*ind));
        vec_i.push_back(nrows);

        vector<int> times;
        for(int ind=0; ind < vec_i.size()-1; ind++)
            times.push_back(vec_i[ind+1]-vec_i[ind]);

        for(int ind=0; ind < times.size(); ind++)
        {
            for(int j=0; j < times[ind]; j++)
                folds.push_back(ind);
        }

        //std::srand ( unsigned ( time(0) ) );
        std::srand( 10 );
        std::random_shuffle ( folds.begin(), folds.end() );
    }

    return folds;
}

int main(int argc, const char* argv[]) {

	if(argc < 2) {
        cerr << "usage: " << argv[0] << " (filename)" << endl;
        exit(EXIT_FAILURE);
	}

	int nrows, ncols;
	cv::Mat data = loadDataFromFile(argv[1]);
	nrows = data.rows; ncols = data.cols;
	cout << "nrows: " << nrows << " ncols: " << ncols << endl;
	for(int i = 0; i < 10; i++) {
		for(int j=0; j < ncols; j++) {
			cout << data.at<double>(i, j) << " ";
		}
		cout << endl;
	}

    int k = 10;
    vector<int> folds = kfolds(nrows, k);

	double err = runDTreeClassification(data, folds);

	cout << "avg err: " << err << endl;

	return 0;
}

#endif // BUILD_TEST