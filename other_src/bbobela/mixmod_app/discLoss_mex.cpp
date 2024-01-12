#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>

#include <mixmod.h>
#include <mixmod/Utilities/ExampleDataUtil.h>

using namespace std;
using namespace XEM;

//#define PRINT_DEBUG

double runDiscriminantAnalysis( const double* data, const int* label, const int nrows, const int ncols, 
                                const vector<int> folds, bool quadratic=false) {

    // run 10 folds
    double total_err = 0;
    for(int k=0; k < 10; k++) {
#ifdef PRINT_DEBUG
        cout << "fold " << k << "/10" << endl;
#endif
        int ntest = std::count (folds.begin(), folds.end(), k);
        int ntrain = folds.size() - ntest;
       
        double** traindata;
        traindata = new double*[ntrain];
      
        double** testdata;
        testdata = new double*[ntest];
      
        vector<int64_t> trainlabel;
        vector<int64_t> testlabel;

        int test_ind = 0, train_ind = 0;
        for(int i=0; i < nrows; i++) {
            if(folds[i] == k) { // test
                //for(int j=0; j < ncols; j++)
                //    testdata[test_ind][j] = data[i*ncols +j];
                testdata[test_ind] = (double*)(data + (i*ncols));
                testlabel.push_back(label[i]);
                test_ind++;
            }
            else { //train
                //for(int j=0; j < ncols; j++)
                //    traindata[train_ind][j] = data[i*ncols +j];
                traindata[train_ind] = (double*)(data + (i*ncols));
                trainlabel.push_back(label[i]);
                train_ind++;
            }
        }

        //train data
        GaussianData* gdata = new GaussianData (ntrain, ncols, traindata);
        DataDescription* dataTrainDescription = new DataDescription (gdata);
        delete gdata ;
        LabelDescription* labelTrainDescription = new LabelDescription (ntrain, trainlabel);

        LearnInput* lInput =  new LearnInput (dataTrainDescription, labelTrainDescription);
        delete dataTrainDescription;
        delete labelTrainDescription;

        // custom model types
        lInput->removeModelType ( 0 );

        // add another model type
        if(!quadratic) // LDA
            lInput->addModel ( XEM::Gaussian_pk_L_C );
        else // QDA
            lInput->addModel ( XEM::Gaussian_pk_Lk_Ck );

        // Finalize input: run a series of sanity checks on it
        lInput->finalize();

        // 2.1. Create XEM::ClusteringMain
        XEM::LearnMain lMain ( lInput );

        // 2.2 run XEM::LearnMain
        lMain.run();
        
        //2.3. Create a new XEM::LearnOutput object
        XEM::LearnOutput * lOutput = lMain.getLearnOutput();

        // XEM::PredictInput
        XEM::LearnModelOutput * lMOutput = NULL;
        lMOutput = lOutput->getLearnModelOutput().front();

        //test data
        GaussianData* gtestdata = new GaussianData (ntest, ncols, testdata);
        DataDescription* dataTestDescription = new DataDescription (gtestdata);
        delete gtestdata ;
        ParameterDescription* paramPredict = new ParameterDescription (lMOutput);
        PredictInput* pInput = new PredictInput (dataTestDescription, paramPredict);
        delete dataTestDescription;
        pInput->finalize();

        // XEM::PredictMain
        XEM::PredictMain pMain ( pInput );
        pMain.run();

        // XEM::PredictOutput
        XEM::PredictOutput * pOutput = pMain.getPredictOutput();
        // get prediction model
        XEM::PredictModelOutput * pMOutput = pOutput->getPredictModelOutput().front();

        // print out labels
        double err = pMOutput->getLabelDescription()->getLabel()->getErrorRate(testlabel);
#ifdef PRINT_DEBUG
        cout << "err: " << err << endl;
#endif
        total_err += err;

        delete lInput;
        delete pInput;
        delete paramPredict;
        
        trainlabel.clear();
        testlabel.clear();
        delete [] traindata;
        delete [] testdata;
     
    } // end k

    total_err /= 10; // k = 10

    try {
        int* a = NULL;
        a[3] = 10;
        delete []a;
    }
    catch (std::exception& e) {
        std::string msg = std::string("Aw snap: ") + e.what();
        std::cout << msg << std::endl;
        throw;
    }

    

    return total_err;
}


#ifndef BUILD_TEST

#include <matrix.h>
#include <mex.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    //avgloss = discLoss_mex(double** X, double* label, double* folds, double type {0: linear, 1: quadratic}) 

    //mexPrintf("discLoss_mex 10-fold\n");

    size_t nrows, ncols;
    double* mat_X;      // col-wise matrix
    double* mat_label;
    double* mat_folds;
  
    if(nrhs != 4) {
        mexPrintf("Incorrect input arguments\n");
        mexPrintf("Usage: avgloss = discLoss_mex(double** X, double* label, double* folds, double type {0: linear, 1: quadratic}). X row-wise matrix\n");
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
    int type = (int)mxGetScalar(prhs[3]);
#ifdef PRINT_DEBUG
    mexPrintf("type: %d\n", type);
#endif
    if(type != 0 && type !=1) {
        mexPrintf("Error: invalid type\n");
        return;
    }

    int* label = new int[nrows];
    set<int> class_labels;

    for(int i=0; i < nrows; i++) {
        class_labels.insert((int)mat_label[i]);
    }

    int num_classes = class_labels.size();
    for(int i=0; i < nrows; i++) {
        for(int j = 0; j < num_classes; j++) {
            int c = *std::next(class_labels.begin(), j);
            if(c == (int)mat_label[i]) {
                label[i] = j+1;
                break;
            }
        }
    }  

    vector<int> folds;
    for(int i=0; i < nrows; i++)
        folds.push_back((int)mat_folds[i]);
   
    double avgerr = 0;

    try {
        if(type == 0)
            avgerr = runDiscriminantAnalysis(mat_X, label, nrows, ncols, folds);
        else
            avgerr = runDiscriminantAnalysis(mat_X, label, nrows, ncols, folds, true);
    }
    catch (std::exception& e) {
        std::string msg = std::string("Aw snap: ") + e.what();
        mexErrMsgTxt(msg.c_str());
        throw;
    }
    
    plhs[0] = mxCreateDoubleScalar(avgerr);

    class_labels.clear();
    folds.clear();
    delete []label;
    
    //mexPrintf("Done!\n");
}


#else // BUILD_TEST

#include <sys/time.h>
unsigned int getTime() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

double* loadDataFromFile(const char* filename, int& nrows, int& ncols) {

    ifstream infile(filename);
    if(!infile) {
        cout << "Error: cannot open file " << filename << " to read" << endl;
        return NULL;
    }
  
    string line;

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

    double* data = new double[nrows*ncols];
    
    int line_index = 0;
    while (getline(infile, line)) {

        if (line.length() == 0)
            continue;

        stringstream ss(line);
        string token;
        int ind = 0;
        while ( ss >> token ) {
            data[line_index*ncols + ind] = atof(token.c_str());
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

int main ( int argc, char* argv[] ) {

    if(argc < 2) {
        cerr << "usage: " << argv[0] << " (filename)" << endl;
        exit(EXIT_FAILURE);
    }

    double* data_all = NULL;
    int nrows, ncols;
    data_all = loadDataFromFile(argv[1], nrows, ncols);
    if(!data_all)
        return 1;

    int N = ncols - 1;
    double* data = new double[nrows*N];
    int* label = new int[nrows];
    set<int> class_labels;

    for(int i=0; i < nrows; i++) {
        for(int j=0; j < N; j++)
            data[i*N+j] = data_all[i*ncols+j];
        int c = (int)data_all[i*ncols+N];
        class_labels.insert(c);
    }

    int num_classes = class_labels.size();
    for(int i=0; i < nrows; i++) {
        for(int j = 0; j < num_classes; j++) {
            int c = *std::next(class_labels.begin(), j);
            if(c == (int)data_all[i*ncols+N]) {
                label[i] = j+1;
                break;
            }
        }
    }

    int k = 10;
    vector<int> folds = kfolds(nrows, k);

    unsigned int t_start = getTime();
    double err = runDiscriminantAnalysis(data, label, nrows, N, folds);
    cout << "total err: " << err << endl;
    cout << "procesing time: " << getTime() - t_start << endl;
    
    delete []data_all;
    delete []data;
    delete []label;

    return 0;
}

#endif // BUILD_TEST