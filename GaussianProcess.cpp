#include <vnl/algo/vnl_cholesky.h>

class MultiGaussian {
public:
    Vector mean;
    Matrix covariance;

    MultiGaussian(Vector const& mean, Matrix const& covariance):
	mean(mean), covariance(covariance) {}
};

template <class T>
class GaussianProcess {
public:
    typedef double (*CovarianceFunction)(const T&, const T&);
private:
    vector<T> inputs;
    CovarianceFunction covariancefunction;
private:
    Matrix L;
    Vector alpha;

    Matrix getcovariancematrix(vector<T> const& in1, 
			       vector<T> const& in2) const
    {
	Matrix K(in1.size(), in2.size());
	
	for(int i=0; i<in1.size(); i++)
	    for(int j=0; j<in2.size(); j++)
		K(i,j) = covariancefunction(in1[i], in2[j]);

	return K;
    }

public:
    GaussianProcess(vector<T> const& inputs, 
		    Vector const& targets,
		    CovarianceFunction covariancefunction, 
		    double noise=0.0):
    inputs(inputs), covariancefunction(covariancefunction)
    {
	Matrix K = getcovariancematrix(inputs, inputs);	
	for(int i=0; i<inputs.size(); i++)
	    K[i][i] += noise;
	// todo: do we need the "verbose" mode in cholesky?
	vnl_cholesky chol(K);
	L = chol.lower_triangle();
	alpha = chol.solve(targets);
    }

    Vector getmeans(vector<T> const &tests) const {
	Matrix KK = getcovariancematrix(inputs, tests);
	return KK.transpose() * alpha;
    }

    double getmean(T const& test) const {
	return getmeans(vector<T>(1, test))[0];
    }
};

template <class T> 
class MeanAdjustedGaussianProcess {
    double mean;
    const GaussianProcess<T> gp;

public:
    MeanAdjustedGaussianProcess(vector<T> const& inputs, 
				Vector const& targets,
				typename GaussianProcess<T>::CovarianceFunction 
				  covariancefunction, 
				double noise=0.0): 
	mean(targets.mean()), 
	gp(GaussianProcess<T>(inputs, targets - mean, 
			      covariancefunction, noise))
    {}   

    double getmean(T const& test) const {
	return gp.getmean(test) + mean;
    }
};
