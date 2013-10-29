/* mir.c
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

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<gsl/gsl_blas.h>
#include<gsl/gsl_linalg.h>
#include<gsl/gsl_matrix.h>
#include<gsl/gsl_permutation.h>
#include<gsl/gsl_permute_vector.h>
#include<gsl/gsl_vector.h>

#include"mir.h"


double factorial(int *kappa, int ndim)
{
    int i;
    double result;

    /* Sanity check on arguments */
    if (ndim <= 0) return 0.0;

    result = 1.0;
    if (ndim > 1) {
        for (i = 0; i < ndim; ++i) {
            result *= factorial(kappa + i, 1);
        }
    }
    else {
        if (kappa[0] < 0) return 0.0;
        for (i = 2; i <= kappa[0]; ++i) {
            result *= i;
        }
    }
    return result;
}


int binomialInv(int l, int n)
{
    int m, b;
    m = n;
    b = 1;
    while (b <= l) {
        m += 1;
        b *= m;
        b /= (m - n);
    }
    return m - 1;
}


int multiIndex(int ndim, int N, int* indices, int lda)
{
    int i, j, total, nsub;

    /* Sanity check on arguments */
    if (ndim <= 0 || N < 0) {
        printf("multiIndex failed, ndim = %d, N = %d\n", ndim, N);
        return -1;
    }
    if (indices != NULL && lda < ndim) return -1;

    if (indices == NULL) {
        /* count only */
        if (ndim == 1) {
            return 1;
        }
        else {
            total = 0;
            for (i = 0; i <= N; ++i) {
                nsub = multiIndex(ndim - 1, i, NULL, 0);
                if (nsub < 0) {
                    printf("multiIndex failed, ndim = %d, N = %d\n", ndim, N);
                    return -1;
                }
                total += nsub;
            }
            return total;
        }
    }
    else {
        /* count and compute, save in indices */
        if (ndim == 1) {
            indices[0] = N;
            return 1;
        }
        else {
            total = 0;
            for (i = 0; i <= N; ++i) {
                nsub = multiIndex(ndim - 1, i, indices + total * lda, lda);
                if (nsub < 0) {
                    printf("multiIndex failed, ndim = %d, N = %d\n", ndim, N);
                    return -1;
                }
                for (j = total; j < total + nsub; ++j) {
                    indices[j * lda + ndim - 1] = N - i;
                }
                total += nsub;
            }
            return total;
        }
    }
}


double weightKappa(int *kappa, int ndim, double beta, double gamma)
{
    int i, kappa_sum;

    kappa_sum = 0;
    for (i = 0; i < ndim; ++i) {
        kappa_sum += kappa[i];
    }
    return beta * pow(gamma, kappa_sum);
}


int mirEvaluate(int nfunc, int ndim, int nx, double* x,
                int nv, double* xv, double* fv, double* sigv,
                int ng, double* xg, double* fg, double* sigg,
                double beta, double gamma, int N, int P, double* fx,
                double* sigma)
{
    int i, ifunc, ix, ierr;
    double *a, *av, *ag, *fvi, *fgi;

    a = (double*) malloc(sizeof(double) * nx * (nv + ng * ndim));
    av = a;
    ag = a + nx * nv;
    ierr = mirBasis(ndim, nx, x, nv, xv, sigv, ng, xg, sigg,
                    beta, gamma, N, P, av, ag, sigma);
    if (ierr < 0) return -1;

    for (ix = 0; ix < nx; ++ix) {
        fvi = fv;
        fgi = fg;
        for (ifunc = 0; ifunc < nfunc; ++ifunc) {
            fx[ifunc] = 0.0;
            for (i = 0; i < nv; ++i) {
                fx[ifunc] += fvi[i] * av[i];
            }
            for (i = 0; i < ng * ndim; ++i) {
                fx[ifunc] += fgi[i] * ag[i];
            }
            fvi += nv;
            fgi += ng * ndim;
        }
        fx += nfunc;
        av += nv;
        ag += ng * ndim;
    }
    free(a);

    return 0;
}


int mirBasis(int ndim, int nx, double* x,
             int nv, double* xv, double* sigv,
             int ng, double* xg, double* sigg,
             double beta, double gamma, int N, int P,
             double* av, double* ag, double* sigma)
{
    int i, j, k, ix, icol, sub, ierr, n = 0, nP = 0;
    int nmi1, nmi2, kappa_n;
    double kappa_coef, factorial_kappa;
    int *indices, *kappa;
    double *X, *X_row, *xv_i, *xg_i;

    /* Sanity check on arguments */
    if ((ndim <= 0) || (nv <= 0) || (ng < 0) || (beta <= 0) ||
        (gamma <= 0) || (N <= 0) || (P < 0 || P > N) || (av == NULL) ||
        (ng != 0 && ag == NULL)) {
            printf("Error: Wrong arguments for mirBasis:\n");
            printf("ndim = %d\nnv = %d\nng = %d\nbeta = %f\ngamma = %f\n",
                   ndim, nv, ng, beta, gamma);
            printf("N = %d\nP = %d\nNull av = %d\nNull ag = %d\n",
                   N, P, av == NULL, ag == NULL);
            return -1;
    }

    /* Calculate multi-indices */
    nmi1 = 0;
    for (i = 0; i < N; ++i) {
        if (i == P) nP = nmi1;
        sub = multiIndex(ndim, i, NULL, 0);
        if (sub < 0) return -1;
        nmi1 += sub;
    }
    if (P == N) nP = nmi1;
    nmi2 = multiIndex(ndim, N, NULL, 0);
    if (nmi2 < 0) return -1;
    indices = (int*) malloc(sizeof(int) * ndim * (nmi1 + nmi2));
    kappa = indices;
    for (i = 0; i <= N; ++i) {
        sub = multiIndex(ndim, i, kappa, ndim);
        if (sub < 0) return -1;
        kappa += ndim * sub;
    }

    X = (double*) malloc(sizeof(double) * (nv + ng * ndim) * (nmi1 + nmi2 + 1));

    for (ix = 0; ix < nx; ++ix) {
        /* Construct matrix X */
        for (k = 0; k < nmi1 + nmi2; ++k) {
            kappa = indices + k * ndim;
            factorial_kappa = factorial(kappa, ndim);
            if (factorial_kappa == 0.0) return -1;
            kappa_coef = weightKappa(kappa, ndim, beta, gamma)
                       / factorial_kappa;
            X_row = X + k * (nv + ng * ndim);
            for (i = 0; i < nv; ++i) {
                xv_i = xv + i * ndim;
                X_row[i] = kappa_coef;
                for (n = 0; n < ndim; ++n) {
                    X_row[i] *= pow(xv_i[n] - x[n], kappa[n]);
                }
            }
        
            for (i = 0; i < ng; ++i) {
                xg_i = xg + i * ndim;
                for (j = 0; j < ndim; ++j) {
                    icol = nv + i * ndim + j;
                    X_row[icol] = kappa[j] * kappa_coef;
                    if (kappa[j] > 0) {
                        for (n = 0; n < ndim; ++n) {
                            kappa_n = (n == j) ? kappa[n] - 1 : kappa[n];
                            X_row[icol] *= pow(xg_i[n] - x[n], kappa_n);
                        }
                    }
                }
            }
        }

        /* Last line of X contains measurement error */
        X_row = X + (nmi1 + nmi2) * (nv + ng * ndim);
        for (i = 0; i < nv; ++i) {
            X_row[i] = sigv[i];
        }
        for (i = 0; i < ng; ++i) {
            for (j = 0; j < ndim; ++j) {
                X_row[nv + i * ndim + j] = sigg[j];
            }
        }

        /* Solve for av and ag */
        ierr = mirSolveAvAg(nv, ng * ndim, nmi1, nmi2, nP, beta, X, av, ag,
                            sigma);
        if (ierr < 0) {
            printf("Error: mirSolveAvAg\n");
            return -1;
        }

        /* Move to the next approximation point */
        x += ndim;
        av += nv;
        ag += ng * ndim;
        sigma += 1;
    }

    free(indices);
    free(X);

    return 0;
}


int mirSolveAvAg(int nv, int ngd, int nmi1, int nmi2, int nP, double beta,
                 double* X, double* av, double* ag, double* sigma)
{
    int i, k, indQ, na, ierr;
    double xi;
    gsl_vector *a, *d, *lambda;
    gsl_matrix *Q;
    gsl_matrix_view A, C;

    na = nv + ngd;
    if (nP <= 0 || nP > na) return -1;

    /* Construct matrix */
    Q = gsl_matrix_alloc(nmi1 + na, na);
    memcpy(Q->data, X, sizeof(double) * nmi1 * na);
    memset(Q->data + nmi1 * na, 0, sizeof(double) * na * na);
    for (i = 0; i < nv + ngd; ++i) {
        indQ = nmi1 * na + i * (na + 1);
        for (k = nmi1; k <= nmi1 + nmi2; ++k) {
            xi = X[k * na + i];
            Q->data[indQ] += xi * xi;
        }
        Q->data[indQ] = sqrt(Q->data[indQ]);
    }

    C = gsl_matrix_submatrix(Q, 0, 0, nP, Q->size2);
    A = gsl_matrix_submatrix(Q, nP, 0, Q->size1 - nP, Q->size2);

    a = gsl_vector_alloc(na);
    d = gsl_vector_calloc(nP);
    d->data[0] = beta;
    lambda = gsl_vector_alloc(nP);

    ierr = lseShurComplement(&A.matrix, &C.matrix, NULL, d, a, lambda, sigma);
    if (ierr < 0) {
        printf("Error: lseShurComplement\n");
        return -1;
    }

    /* Copy solution to output array */
    memcpy(av, a->data, sizeof(double) * nv);
    memcpy(ag, a->data + nv, sizeof(double) * ngd);

    gsl_vector_free(lambda);
    gsl_vector_free(a);
    gsl_vector_free(d);
    gsl_matrix_free(Q);

    return 0;
}


int lsQRPT(gsl_matrix * A, gsl_vector * b, gsl_vector * x, double * sigma)
{
    int i;
    gsl_vector *tau, *res;
    gsl_permutation *p;
    gsl_vector_view norm;

    if (A->size1 < A->size2) return -1;
    if (A->size1 != b->size) return -1;
    if (A->size2 != x->size) return -1;

    tau = gsl_vector_alloc(x->size);
    res = gsl_vector_alloc(b->size);
    p = gsl_permutation_alloc(x->size);
    norm = gsl_vector_subvector(res, 0, x->size);
    gsl_linalg_QRPT_decomp(A, tau, p, &i, &norm.vector);
    gsl_linalg_QR_lssolve(A, tau, b, x, res);
    gsl_permute_vector_inverse(p, x);
    *sigma = gsl_blas_dnrm2(res);

    gsl_vector_free(tau);
    gsl_vector_free(res);
    gsl_permutation_free(p);

    return 0;
}

int lseShurComplement(gsl_matrix * A, gsl_matrix * C,
                      gsl_vector * b, gsl_vector * d,
                      gsl_vector * x, gsl_vector * lambda, double * sigma)
{
    int i;
    double xi;
    gsl_vector *c0, *S, *tau;
    gsl_matrix *CT, *U;
    gsl_permutation *perm;
    gsl_vector_view row, cp;
    gsl_matrix_view R;

    if (A->size2 != C->size2) return -1;
    if (A->size2 != x->size) return -1;
    if (A->size1 < A->size2) return -1;
    if (b != NULL && A->size1 != b->size) return -1;
    if (C->size1 != d->size) return -1;
    if (C->size1 != lambda->size) return -1;

    c0 = gsl_vector_alloc(x->size);
    gsl_matrix_get_row(c0, C, 0);

    /* Cholesky factorization of A^T A = R^T R via QRPT decomposition */
    perm = gsl_permutation_alloc(x->size);
    tau = gsl_vector_alloc(x->size);
    gsl_linalg_QRPT_decomp(A, tau, perm, &i, x);

    /* cp = R^{-T} P A^T b = Q^T b */
    if (b != NULL) {
        gsl_linalg_QR_QTvec(A, tau, b);
        cp = gsl_vector_subvector(b, 0, x->size);
    }
    gsl_vector_free(tau);

    /* C P -> C */
    R = gsl_matrix_submatrix(A, 0, 0, A->size2, A->size2);
    for (i = 0; i < C->size1; ++i) {
        row = gsl_matrix_row(C, i);
        gsl_permute_vector(perm, &row.vector);
    }

    /* Compute C inv(R) -> C */
    gsl_blas_dtrsm(CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit, 1.0,
                   &R.matrix, C);

    /* The Schur complement D = C C^T,
       Compute SVD of D = U S^2 U^T by SVD of C^T = V S U^T */
    CT = gsl_matrix_alloc(C->size2, C->size1);
    gsl_matrix_transpose_memcpy(CT, C);
    U = gsl_matrix_alloc(CT->size2, CT->size2);
    S = gsl_vector_alloc(CT->size2);
    gsl_linalg_SV_decomp(CT, U, S, lambda);

    /* Right hand side of the Shur complement system
       d - C (A^T A)^-1 A^T b = d - C cp -> d
       (with C P R^-1 -> C and R^-T P^T A^T b -> cp) */
    if (b != NULL) {
        gsl_blas_dgemv(CblasNoTrans, -1.0, C, &cp.vector, 1.0, d);
    }

    /* Calculate S U^T lambda, where -lambda is the Lagrange multiplier */
    gsl_blas_dgemv(CblasTrans, 1.0, U, d, 0.0, lambda);
    gsl_vector_div(lambda, S);

    /* Calculate sigma = || A x ||_2 = || x ||_2 (before inv(R) x -> x) */
    *sigma = gsl_blas_dnrm2(lambda);

    /* Compute inv(R)^T C^T lambda = C^T lambda (with C inv(R) ->C) */
    gsl_blas_dgemv(CblasNoTrans, 1.0, CT, lambda, 0.0, x);

    /* x = inv(A^T A) C^T lambda = inv(R) [inv(R)^T C^T lambda] */
    if (R.matrix.data[R.matrix.size1 * R.matrix.size2 - 1] != 0.0) {
        gsl_blas_dtrsv(CblasUpper, CblasNoTrans, CblasNonUnit, &R.matrix, x);
    }
    else {  /* Special case when A is singular */
        gsl_vector_set_basis(x, x->size - 1);
        *sigma = 0.0;
    }

    /* Permute back, 1-step iterative refinement on first constraint */
    gsl_permute_vector_inverse(perm, x);
    gsl_blas_ddot(x, c0, &xi);
    gsl_vector_scale(x, d->data[0] / xi);

    /* get the real lambda from S U^T lambda previously stored in lambda */
    gsl_vector_div(lambda, S);
    gsl_vector_memcpy(S, lambda);
    gsl_blas_dgemv(CblasNoTrans, 1.0, U, S, 0.0, lambda);

    gsl_vector_free(c0);
    gsl_vector_free(S);
    gsl_matrix_free(U);
    gsl_matrix_free(CT);
    gsl_permutation_free(perm);

    return 0;
}


int mirGammaBounds(int ndim, int nv, double* xv, int ng, double* xg,
                   double* minGamma, double* maxGamma)
{
    const double PI = atan(1.0) * 4;

    int i, j, k;
    double delta, deltaSqr, minDeltaSqr, maxDeltaSqr;
    double *xi, *xj;

    if (nv <= 1) return -1;

    /* maximum distance between any two data points, largest length scale */
    maxDeltaSqr = 0.0;
    for (i = 1; i < nv + ng; ++i) {
        xi = (i < nv) ? xv + i * ndim : xg + (i - nv) * ndim;
        for (j = 0; j < i; ++j) {
            xj = (j < nv) ? xv + j * ndim : xg + (j - nv) * ndim;
            deltaSqr = 0.0;
            for (k = 0; k < ndim; ++k) {
                delta = (xi[k] - xj[k]);
                deltaSqr += delta * delta;
            }
            maxDeltaSqr = (deltaSqr > maxDeltaSqr) ? deltaSqr : maxDeltaSqr;
        }
    }

    /* minimum distance between any two value / gradient data points, smallest
       length scale */
    minDeltaSqr = 1.0 / 0.0;
    for (i = 1; i < nv; ++i) {
        xi = xv + i * ndim;
        for (j = 0; j < i; ++j) {
            xj = xv + j * ndim;
            deltaSqr = 0.0;
            for (k = 0; k < ndim; ++k) {
                delta = (xi[k] - xj[k]);
                deltaSqr += delta * delta;
            }
            minDeltaSqr = (deltaSqr < minDeltaSqr) ? deltaSqr : minDeltaSqr;
        }
    }
    for (i = 1; i < ng; ++i) {
        xi = xg + i * ndim;
    /* maximum distance between any two data points */
        for (j = 0; j < i; ++j) {
            xj = xg + j * ndim;
            deltaSqr = 0.0;
            for (k = 0; k < ndim; ++k) {
                delta = (xi[k] - xj[k]);
                deltaSqr += delta * delta;
            }
            minDeltaSqr = (deltaSqr < minDeltaSqr) ? deltaSqr : minDeltaSqr;
        }
    }

    *minGamma = 1. / sqrt(maxDeltaSqr);
    *maxGamma = PI / sqrt(minDeltaSqr);

    return 0;
}


int mirBetaGamma(int nfunc, int ndim,
                 int nv, double* xv, double* fv, double* sigv,
                 int ng, double* xg, double* fg, double* sigg,
                 int N, int P, double safety, double* beta, double* gamma)
{
    int i, j, n, ierr;
    double fvMean, dfv, gammaMin, gammaMax, resRatio, sigma, resid, resSqr;
    double *xvp, *fvp, *sigvp, *fx;

    /* Calculate default beta */
    *beta = 0.0;
    for (n = 0; n < nfunc; ++n) {
        fvMean = 0.0;
        for (i = 0; i < nv; ++i) {
            fvMean += fv[n * nv + i];
        }
        fvMean /= nv;
        for (i = 0; i < nv; ++i) {
            dfv = fv[n * nv + i] - fvMean;
            *beta += dfv * dfv;
        }
    }
    *beta = sqrt(*beta / (nv - 1) / nfunc);

    /* Calculate bounds for default gamma */
    ierr = mirGammaBounds(ndim, nv, xv, ng, xg, &gammaMin, &gammaMax);
    if (ierr < 0) return -1;

    /* Bisection */
    fx = (double*) malloc(sizeof(double) * nfunc);
    xvp = (double*) malloc(sizeof(double) * nv * ndim);
    fvp = (double*) malloc(sizeof(double) * nv);
    sigvp = (double*) malloc(sizeof(double) * nv);
    memcpy(xvp, xv + ndim, sizeof(double) * (nv - 1) * ndim);
    memcpy(fvp, fv + 1, sizeof(double) * (nv - 1));
    memcpy(sigvp, sigv + 1, sizeof(double) * (nv - 1));

    while (gammaMax / gammaMin > 1.1) {
        *gamma = sqrt(gammaMax * gammaMin);
        /* First sub-mir */
        memcpy(xvp, xv + ndim, sizeof(double) * (nv - 1) * ndim);
        memcpy(fvp, fv + 1, sizeof(double) * (nv - 1));
        memcpy(sigvp, sigv + 1, sizeof(double) * (nv - 1));
        /* Calculate ratio of true residual to estimated residual */
        resRatio = 0.0;
        for (i = 0; i < nv; ++i) {
            ierr = mirEvaluate(nfunc, ndim, 1, xv + i * ndim,
                               nv - 1, xvp, fvp, sigvp, ng, xg, fg, sigg,
                               *beta, *gamma, N, P, fx, &sigma);
            if (ierr < 0) {
                printf("mirEvaluate failed at data point %d in mirBetaGamma\n",
                       i);
                return -1;
            }
            resSqr = 0.0;
            for (j = 0; j < nfunc; ++j) {
                resid = fx[j] - fv[i + j * nv];
                resSqr += resid * resid;
            }
            resRatio += resSqr / (sigma * sigma + sigv[i] * sigv[i]);
            /* Next sub-mir */
            memcpy(xvp + i * ndim, xv + i * ndim, sizeof(double) * ndim);
            fvp[i] = fv[i];
            sigvp[i] = sigv[i];
        }
        resRatio *= safety / nv / nfunc;
        if (resRatio < 1.0) {
            gammaMax = *gamma;
        }
        else {
            gammaMin = *gamma;
        }
    }
    *gamma = sqrt(gammaMax * gammaMin);

    free(fx);
    free(xvp);
    free(fvp);
    free(sigvp);

    return 0;
}

