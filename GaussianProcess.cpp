#include <numeric>

template <class T> class GaussianProcess {
public:
	typedef double (*CovarianceFunction)(const T&, const T&);
private:
	std::vector<T> _inputs;
	CovarianceFunction _covarianceFunction;
	CvMat *_alpha;

	CvMat *getCovarianceMatrix(std::vector<T> const &in1, std::vector<T> const &in2) const {
		CvMat *K = cvCreateMat(in1.size(), in2.size(), CV_32FC1);

		for (int i = 0; i < in1.size(); i++) {
			for (int j = 0; j < in2.size(); j++) {
				CV_MAT_ELEM(*K, float, i, j) = _covarianceFunction(in1[i], in2[j]);
			}
		}

		return K;
	}

public:
	GaussianProcess(std::vector<T> const &inputs, std::vector<double> const &targets, CovarianceFunction covarianceFunction, double noise=0.0):
		_inputs(inputs),
		_covarianceFunction(covarianceFunction),
		_alpha(cvCreateMat(targets.size(), 1, CV_32FC1))
	{
		CvMat *targetsMatrix = cvCreateMat(targets.size(), 1, CV_32FC1);
		for (int i = 0; i < targets.size(); i++) {
			CV_MAT_ELEM(*targetsMatrix, float, i, 0) = targets[i];
		}

		CvMat* K = getCovarianceMatrix(_inputs, _inputs);
		for (int i = 0; i < inputs.size(); i++) {
			CV_MAT_ELEM(*K, float, i, i) += noise;
		}

		cvSolve(K, targetsMatrix, _alpha, CV_CHOLESKY);
	}

	CvMat* getMeans(std::vector<T> const &tests) const {
		CvMat *KK = getCovarianceMatrix(_inputs, tests);
		CvMat *result = cvCreateMat(KK->cols, _alpha->cols, CV_32FC1);

		cvGEMM(KK, _alpha, 1, NULL, 0, result, CV_GEMM_A_T);

		return result;
	}

	double getMean(T const &test) const {
		CvMat* means = getMeans(std::vector<T>(1, test));
		return CV_MAT_ELEM(*means, float, 0, 0);
	}
};

template <class T> class MeanAdjustedGaussianProcess {
public:
	MeanAdjustedGaussianProcess(std::vector<T> const &inputs, std::vector<double> const &targets, typename GaussianProcess<T>::CovarianceFunction covarianceFunction, double noise=0.0) {
		_mean = std::accumulate(targets.begin(), targets.end(), 0.0) / targets.size();

		// Calculate "targets - mean"
		std::vector<double> zeroMeanTargets;
		for (int i = 0; i < targets.size(); i++) {
			zeroMeanTargets.push_back(targets[i] - _mean);
		}

		_gaussianProcess = new GaussianProcess<T>(inputs, zeroMeanTargets, covarianceFunction, noise);
	}

	double getmean(T const &test) const {
		return _gaussianProcess->getMean(test) + _mean;
	}

private:
	double _mean;
	GaussianProcess<T> *_gaussianProcess;
};
