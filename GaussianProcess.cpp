#include "utils.h"
#include <algorithm>
#include <functional>
#include <numeric>


template <class T>
class GaussianProcess {
public:
    typedef double (*CovarianceFunction)(const T&, const T&);
private:
    vector<T> inputs;
    CovarianceFunction covariancefunction;
private:
    CvMat* alpha;
    
    CvMat* getcovariancematrix(vector<T> const& in1, 
			       vector<T> const& in2) const
    {
        CvMat* K = cvCreateMat(in1.size(), in2.size(), CV_32FC1);
	
    	for(int i=0; i<in1.size(); i++)
    	    for(int j=0; j<in2.size(); j++)
    	    CV_MAT_ELEM( *K, float, i, j ) = covariancefunction(in1[i], in2[j]);

    	return K;
    }

public:
    GaussianProcess(vector<T> const& inputs, 
		    vector<double> const& targets,
		    CovarianceFunction covariancefunction, 
		    double noise=0.0):
    inputs(inputs), covariancefunction(covariancefunction)
    {
    	CvMat* targets_matrix = cvCreateMat(targets.size(), 1, CV_32FC1);
    	alpha = cvCreateMat(targets.size(), 1, CV_32FC1);
	
    	for(int i=0; i<targets.size(); i++)
    	    CV_MAT_ELEM( *targets_matrix, float, i, 0 ) = targets[i];
	
    	CvMat* K = getcovariancematrix(inputs, inputs);
    	for(int i=0; i<inputs.size(); i++)
    	    CV_MAT_ELEM( *K, float, i, i ) += noise;
    	
        cvSolve(K, targets_matrix, alpha, CV_CHOLESKY);
    }

    CvMat* getmeans(vector<T> const &tests) const {
    	CvMat* KK = getcovariancematrix(inputs, tests); 
        CvMat* result = cvCreateMat(KK->cols, alpha->cols, CV_32FC1);
  
    	cvGEMM(KK, alpha, 1, NULL, 0, result, CV_GEMM_A_T);

        return result;
    }

    double getmean(T const& test) const {
        CvMat* means = getmeans(vector<T>(1, test));
        return CV_MAT_ELEM( *means, float, 0, 0 );
    }
};

template <class T> 
class MeanAdjustedGaussianProcess {
    double mean;
    GaussianProcess<T>* gp;

public:
    MeanAdjustedGaussianProcess(vector<T> const& inputs, 
				vector<double> const& targets,
				typename GaussianProcess<T>::CovarianceFunction 
				  covariancefunction, 
				double noise=0.0) {
				    
	
        mean = std::accumulate(targets.begin(), targets.end(), 0.0) / targets.size();
        
        // Calculate "targets - mean"
        vector<double> zero_mean_targets;
        for(int i=0; i<targets.size(); i++) {
            zero_mean_targets.push_back(targets[i]-mean);
        }
        
        gp = new GaussianProcess<T>(inputs, zero_mean_targets, covariancefunction, noise);
	}

    double getmean(T const& test) const {
	return gp->getmean(test) + mean;
    }
};
