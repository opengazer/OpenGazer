#pragma once
#include <vnl/vnl_vector.h>
#include <vnl/vnl_matrix.h>

typedef vnl_vector<double> Vector;
typedef vnl_matrix<double> Matrix;

class LeastSquares {
    Matrix X;
    Vector Y;
public:
    const int n;

    LeastSquares(int n): X(n,n), Y(n), n(n) {}

    void addSample(double xs[], double y);
    void addSample(double x1, double x2, double y);
    void addSample(double x1, double x2, double x3, double y);

    Vector solve(void);
    void solve(double &a0, double &a1);
    void solve(double &a0, double &a1, double &a2);
};

 
