#include <assert.h>

#include "LeastSquares.h"

LeastSquares::LeastSquares(int n):
	n(n)
{
	X = cvCreateMat(n, n, CV_32FC1);
	Y = cvCreateMat(n, 1, CV_32FC1);
}

void LeastSquares::addSample(double xs[], double y) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			CV_MAT_ELEM(*X, float, i, j) += xs[i] * xs[j];
		}
	}

	for (int i = 0; i < n; i++) {
		CV_MAT_ELEM(*Y, float, i, 0) += xs[i] * y;
	}
}

void LeastSquares::addSample(double x1, double x2, double y) {
	assert(n == 2);
	double xs[2] = {x1, x2};
	addSample(xs, y);
}

void LeastSquares::addSample(double x1, double x2, double x3, double y) {
	assert(n == 3);
	double xs[3] = {x1, x2, x3};
	addSample(xs, y);
}

CvMat* LeastSquares::solve(void) {
	CvMat *result = cvCreateMat(n, 1, CV_32FC1);
	cvSolve(X, Y, result, CV_CHOLESKY);
	return result;
}

void LeastSquares::solve(double &a0, double &a1, double &a2) {
	assert(n == 3);
	CvMat *vec = solve();
	a0 = CV_MAT_ELEM(*vec, float, 0, 0);
	a1 = CV_MAT_ELEM(*vec, float, 1, 0);
	a2 = CV_MAT_ELEM(*vec, float, 2, 0);
}

