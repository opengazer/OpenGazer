/* mir.h
 * 
 * Copyright (C) 2009 Qiqi Wang
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _MIR_H_QIQI_WANG_
#define _MIR_H_QIQI_WANG_

#include<gsl/gsl_blas.h>
#include<gsl/gsl_linalg.h>
#include<gsl/gsl_matrix.h>
#include<gsl/gsl_permutation.h>
#include<gsl/gsl_permute_vector.h>
#include<gsl/gsl_vector.h>

/*
Calculate multivariate factorial.

Input arguments:
    kappa    Array of integers
    ndim     Dimension of kappa
Return value:
    kappa[0]! * kappa[1]! * ... * kappa[ndim-1]!
    In error, return 0.0
*/
double factorial(int *kappa, int ndim);

/*
Calculate the largest m such that binomial(m, n) <= l

Input arguments:
    l, n
Return value:
    largest value of m such that binomial(m, n) <= l
*/
int binomialInv(int l, int n);

/*
Calculate all multi-indices of level N.

Input arguments:
    ndim     Dimension of space
    N        Total order
    lda      Specifies the first dimension of indices (>=N)
Output arguments:
    indices  All multi-indices of level N, lda specifies its first dimension,
             It can be set to NULL, when the subroutine only count the number
             of multi-indices of level N, without computing them.
Return value:
    The number of multi-indices of level N
    In error return -1
*/
int multiIndex(int ndim, int N, int* indices, int lda);

/*
Calculate the weight for a multi-index

Input arguments:
    kappa    Array containing components of the multi-index
    ndim     Length of the multi-index
    beta     The magnitude parameter
    gamma    The wavenumber parameter
Return value:
    The weight of a multi-index
*/
double weightKappa(int *kappa, int ndim, double beta, double gamma);

/*
Calculate Multivariate Interpolation and Regression approximation.

Input arguments:
    nfunc   Number of functions approximated
    ndim    Dimension of the approximation space (>0)
    nx      number of approximation points
    x       ndim * nx dimensional array, the approximation points (*)
    nv      number of value data points (>0)
    xv      ndim * nv dimensional array, value data points (*)
    fv      nv * nfunc dimensional array, function values at value data points
            (**)
    sigv    nv dimensional array, measurement error of value data points
    ng      number of gradient data points (>=0)
    xg      ndim * ng dimensional array, gradient data points (*)
    fg      nfunc * ng * ndim dimensional array, function gradients at
            gradient data points (***)
    sigg    ng dimensional array, measurement error of gradient data points
    beta    The magnitude parameter (>0)
    gamma   The wavenumber parameter (>0)
    N       The Taylor order parameter (>0)
    P       The polynomial exactness parameter (>=0)
Output arguments:
    fx      nx * nfunc dimensional array, the approximation function value
            at each approximation point (**)
    sigma   nx dimensional array, estimated approximation error at each
            approximation point
Return value:
    0 if ok, -1 if error.

(*)   The ndim coordinates of each point should be contiguous in memory.
(**)  The values of the same function should be contiguous in memory.
(***) The ndim components of each gradient vector should be contiguous
      in memory; up one level, the ng gradient vectors of the same
      function should be contiguous in memory; the gradient vectors of
      different functions should be the outer-most loop in memory space.
*/
int mirEvaluate(int nfunc, int ndim, int nx, double* x,
                int nv, double* xv, double* fv, double* sigv,
                int ng, double* xg, double* fg, double* sigg,
                double beta, double gamma, int N, int P, double* fx,
                double* sigma);

int mirReconstruct(int ndim, int nx, double* x, double* fg, int N, int P,
                   double* fv);

/*
Calculate the approximation coefficients, a.k.a. the value of the basis
functions at point x.

Input arguments:
    ndim    Dimension of the approximation space (>0)
    nx      number of approximation points
    x       ndim * nx dimensional array, the approximation points (*)
    nv      number of value data points (>0)
    xv      ndim * nv dimensional array, value data points (*)
    sigv    nv dimensional array, measurement error of value data points
    ng      number of gradient data points (>=0)
    xg      ndim * ng dimensional array, gradient data points (*)
    sigg    ng dimensional array, measurement error of gradient data points
    beta    The magnitude parameter (>0)
    gamma   The wavenumber parameter (>0)
    N       The Taylor order parameter (>0)
    P       The polynomial exactness parameter (>=0)
Output arguments:
    av      nv * nx dimensional array, coefficients of value data points
            on each approximation point (**)
    ag      ndim * ng * nx dimensional array, coefficients of gradient
            data points on each approximation point (***)
    sigma   nx dimensional array,
            estimated approximation error on each approximation point
Return value:
    0 if ok, -1 if error.

(*)   The ndim coordinates of each point should be contiguous in memory.
(**)  The coefficients of the same interpolation point is contiguous in
      memory.
(***) The coefficients of the ndim components of each gradient vector
      is contiguous in memory; up one level, the coefficient vectors of
      the ng gradient data points for the same interpolation point is
      contiguous in memory; the coefficients for different interpolation
      points is the outer-most loop in memory space.
*/
int mirBasis(int ndim, int nx, double* x,
             int nv, double* xv, double* sigv,
             int ng, double* xg, double* sigg,
             double beta, double gamma, int N, int P,
             double* av, double* ag, double* sigma);

/*
Calculate the reconstruction matrix, a.k.a. the value of the basis
functions at each point.

Input arguments:
    ndim    Dimension of the approximation space (>0)
    nx      number of approximation points
    x       ndim * nx dimensional array, the gradient points (*)
    gamma   The wavenumber parameter (>0)
    N       The Taylor order parameter (>0)
    P       The polynomial exactness parameter (>=0)
Output arguments:
    a       nx * nx * ndim dimensional array, the reconstruction
            matrix. (**)
Return value:
    0 if ok, -1 if error.

(*)   The ndim coordinates of each point should be contiguous in memory.
(***) The coefficients of the ndim components of each gradient vector
      is contiguous in memory; up one level, the coefficient vectors of
      the nx gradient data points for the same point is contiguous in
      memory; the coefficients for different points is the outer-most
      loop in memory space.
*/
int mirReconstructBasis(int ndim, int nx, double* x,
                        double gamma, int N, int P, double* a);

/*
Solve constraint least squares for av and ag.

Input arguments:
    nv      length of av, the number of value data points
    ngd     length of ag, equal to ng * ndim
    nmi1    number of terms in Taylor expansion
    nmi2    number of terms in Taylor residual
    nP      number of constraints, determined by polynomial exactness
    X       matrix X, size (nmi1 + nmi2 + 1) by (nv + ngd), each row is
            contiguous in memory
Output arguments:
    av      nv dimensional array, coefficients on value data points
    ag      ngd dimensional array, coefficients on gradient data points
    sigma   estimated approximation error
Return value:
    0 if ok, -1 if error.
*/
int mirSolveAvAg(int nv, int ngd, int nmi1, int nmi2, int nP, double beta,
                 double* X, double* av, double* ag, double* sigma);

/*
Reconstruction a surface from gradient data,
Calculate av from the Taylor residuals only,
Calculate ag by solving least squares.

Input arguments:
    nv      length of av, the number of value data points
    ngd     length of ag, equal to ng * ndim
    nmi1    number of terms in Taylor expansion
    nmi2    number of terms in Taylor residual
    nP      number of constraints, determined by polynomial exactness
    X       matrix X, size (nmi1 + nmi2) by (nv + ngd), each row is
            contiguous in memory
Output arguments:
    av      nv dimensional array, coefficients on value data points
    ag      ngd dimensional array, coefficients on gradient data points
Return value:
    0 if ok, -1 if error.
*/
int mirSolveRecons(int nv, int ngd, int nmi1, int nmi2, int nP,
                   double* X, double* av, double* ag);


/*
Solve equality constrained least squares using Shur complement.
    min || A x - b ||_2 , s.t. C x = d

Input arguments:
    A      must be a tall matrix
    C      must be a fat matrix, with same number of columns as A (*)
    b      length being same as rows of A, NULL stands for 0.
    d      length being same as rows of C
Output arguments:
    x      the solution of the equality constrined least squares
    sigma  the minimum value of L2 norm
Return value:
    0 if ok, -1 if error.
*/
int lseShurComplement(gsl_matrix * A, gsl_matrix * C,
                      gsl_vector * b, gsl_vector * d,
                      gsl_vector * x, gsl_vector * lambda, double * sigma);

/*
Solve un-constrained least squares using QR decomposition with column pivoting
    min || A x - b ||_2

Input arguments:
    A      must be a tall matrix
    b      length being same as rows of A, NULL stands for 0.
Output arguments:
    x      the solution of the least squares
    sigma  the minimum value of L2 norm
Return value:
    0 if ok, -1 if error.
*/
int lsQRPT(gsl_matrix * A, gsl_vector * b, gsl_vector * x, double * sigma);

/*
Calculate the lower and upper bound on parameter gamma based
only on grid points

Input arguments:
    nfunc    Number of functions approximated
    ndim     Dimension of the approximation space (>0)
    nv       number of value data points (>0)
    xv       ndim * nv dimensional array, value data points
    ng       number of gradient data points (>=0)
    xg       ndim * ng dimensional array, gradient data points
Output arguments:
    minGamma The lower bound of the wavenumber parameter gamma
    maxGamma The upper bound of the wavenumber parameter gamma
Return value:
    0 if ok, -1 if error.
*/
int mirGammaBounds(int ndim, int nv, double* xv, int ng, double* xg,
                   double* minGamma, double* maxGamma);

/*
Calculate the best parameters beta and gamma

Input arguments:
    nfunc   Number of functions approximated
    ndim    Dimension of the approximation space (>0)
    nv      number of value data points (>0)
    xv      ndim * nv dimensional array, value data points
    fv      nfunc * nv dimensional array, function values at value data points
    sigv    nv dimensional array, measurement error of value data points
    ng      number of gradient data points (>=0)
    xg      ndim * ng dimensional array, gradient data points
    fg      nfunc * ng * ndim dimensional array, function gradients at
            gradient data points
    sigg    ng dimensional array, measurement error of gradient data points
    N       The Taylor order parameter (>0)
    P       The polynomial exactness parameter (>=0)
    safety  The safety factor, must be greater than 0.
            Higher than 1 will produce a conservative (larger) gamma,
            lower than 1 will produce a aggressive (smaller) gamma.
Output arguments:
    beta    The magnitude parameter
    gamma   The wavenumber parameter
Return value:
    0 if ok, -1 if error.
*/
int mirBetaGamma(int nfunc, int ndim,
                 int nv, double* xv, double* fv, double* sigv,
                 int ng, double* xg, double* fg, double* sigg,
                 int N, int P, double safety, double* beta, double* gamma);

#endif
