#include "LeastSquares.h"
#include <vnl/algo/vnl_cholesky.h>
#include <assert.h>

void LeastSquares::addSample(double xs[], double y) {
    for(int i=0; i<n; i++)
	for(int j=0; j<n; j++)
	    X[i][j] += xs[i]*xs[j];

    for(int i=0; i<n; i++)
	Y[i] += xs[i]*y;
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

Vector LeastSquares::solve(void) {
    return vnl_cholesky(X).solve(Y);
}

void LeastSquares::solve(double &a0, double &a1) {
    assert(n == 2);
    Vector vec = solve();
    a0 = vec[0];
    a1 = vec[1];
}

void LeastSquares::solve(double &a0, double &a1, double &a2) {
    assert(n == 3);
    Vector vec = solve();
    a0 = vec[0];
    a1 = vec[1];
    a2 = vec[2];
}


 
