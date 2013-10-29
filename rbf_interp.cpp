# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <cmath>
# include <ctime>

using namespace std;

# include "rbf_interp.hpp"

//****************************************************************************80

void daxpy ( int n, double da, double dx[], int incx, double dy[], int incy )

//****************************************************************************80
//
//  Purpose:
//
//    DAXPY computes constant times a vector plus a vector.
//
//  Discussion:
//
//    This routine uses unrolled loops for increments equal to one.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    02 May 2005
//
//  Author:
//
//    Original FORTRAN77 version by Charles Lawson, Richard Hanson, 
//    David Kincaid, Fred Krogh.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jack Dongarra, Jim Bunch, Cleve Moler, Pete Stewart,
//    LINPACK User's Guide,
//    SIAM, 1979,
//    ISBN13: 978-0-898711-72-1,
//    LC: QA214.L56.
//
//    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
//    Basic Linear Algebra Subprograms for Fortran Usage,
//    Algorithm 539,
//    ACM Transactions on Mathematical Software,
//    Volume 5, Number 3, September 1979, pages 308-323.
//
//  Parameters:
//
//    Input, int N, the number of elements in DX and DY.
//
//    Input, double DA, the multiplier of DX.
//
//    Input, double DX[*], the first vector.
//
//    Input, int INCX, the increment between successive entries of DX.
//
//    Input/output, double DY[*], the second vector.
//    On output, DY[*] has been replaced by DY[*] + DA * DX[*].
//
//    Input, int INCY, the increment between successive entries of DY.
//
{
  int i;
  int ix;
  int iy;
  int m;

  if ( n <= 0 )
  {
    return;
  }

  if ( da == 0.0 )
  {
    return;
  }
//
//  Code for unequal increments or equal increments
//  not equal to 1.
//
  if ( incx != 1 || incy != 1 )
  {
    if ( 0 <= incx )
    {
      ix = 0;
    }
    else
    {
      ix = ( - n + 1 ) * incx;
    }

    if ( 0 <= incy )
    {
      iy = 0;
    }
    else
    {
      iy = ( - n + 1 ) * incy;
    }

    for ( i = 0; i < n; i++ )
    {
      dy[iy] = dy[iy] + da * dx[ix];
      ix = ix + incx;
      iy = iy + incy;
    }
  }
//
//  Code for both increments equal to 1.
//
  else
  {
    m = n % 4;

    for ( i = 0; i < m; i++ )
    {
      dy[i] = dy[i] + da * dx[i];
    }

    for ( i = m; i < n; i = i + 4 )
    {
      dy[i  ] = dy[i  ] + da * dx[i  ];
      dy[i+1] = dy[i+1] + da * dx[i+1];
      dy[i+2] = dy[i+2] + da * dx[i+2];
      dy[i+3] = dy[i+3] + da * dx[i+3];
    }

  }

  return;
}
//****************************************************************************80

double ddot ( int n, double dx[], int incx, double dy[], int incy )

//****************************************************************************80
//
//  Purpose:
//
//    DDOT forms the dot product of two vectors.
//
//  Discussion:
//
//    This routine uses unrolled loops for increments equal to one.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    02 May 2005
//
//  Author:
//
//    Original FORTRAN77 version by Charles Lawson, Richard Hanson, 
//    David Kincaid, Fred Krogh.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jack Dongarra, Jim Bunch, Cleve Moler, Pete Stewart,
//    LINPACK User's Guide,
//    SIAM, 1979,
//    ISBN13: 978-0-898711-72-1,
//    LC: QA214.L56.
//
//    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
//    Basic Linear Algebra Subprograms for Fortran Usage,
//    Algorithm 539,
//    ACM Transactions on Mathematical Software,
//    Volume 5, Number 3, September 1979, pages 308-323.
//
//  Parameters:
//
//    Input, int N, the number of entries in the vectors.
//
//    Input, double DX[*], the first vector.
//
//    Input, int INCX, the increment between successive entries in DX.
//
//    Input, double DY[*], the second vector.
//
//    Input, int INCY, the increment between successive entries in DY.
//
//    Output, double DDOT, the sum of the product of the corresponding
//    entries of DX and DY.
//
{
  double dtemp;
  int i;
  int ix;
  int iy;
  int m;

  dtemp = 0.0;

  if ( n <= 0 )
  {
    return dtemp;
  }
//
//  Code for unequal increments or equal increments
//  not equal to 1.
//
  if ( incx != 1 || incy != 1 )
  {
    if ( 0 <= incx )
    {
      ix = 0;
    }
    else
    {
      ix = ( - n + 1 ) * incx;
    }

    if ( 0 <= incy )
    {
      iy = 0;
    }
    else
    {
      iy = ( - n + 1 ) * incy;
    }

    for ( i = 0; i < n; i++ )
    {
      dtemp = dtemp + dx[ix] * dy[iy];
      ix = ix + incx;
      iy = iy + incy;
    }
  }
//
//  Code for both increments equal to 1.
//
  else
  {
    m = n % 5;

    for ( i = 0; i < m; i++ )
    {
      dtemp = dtemp + dx[i] * dy[i];
    }

    for ( i = m; i < n; i = i + 5 )
    {
      dtemp = dtemp + dx[i  ] * dy[i  ]
                    + dx[i+1] * dy[i+1]
                    + dx[i+2] * dy[i+2]
                    + dx[i+3] * dy[i+3]
                    + dx[i+4] * dy[i+4];
    }

  }

  return dtemp;
}
//****************************************************************************80

double dnrm2 ( int n, double x[], int incx )

//****************************************************************************80
//
//  Purpose:
//
//    DNRM2 returns the euclidean norm of a vector.
//
//  Discussion:
//
//     DNRM2 ( X ) = sqrt ( X' * X )
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    02 May 2005
//
//  Author:
//
//    Original FORTRAN77 version by Charles Lawson, Richard Hanson, 
//    David Kincaid, Fred Krogh.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jack Dongarra, Jim Bunch, Cleve Moler, Pete Stewart,
//    LINPACK User's Guide,
//    SIAM, 1979,
//    ISBN13: 978-0-898711-72-1,
//    LC: QA214.L56.
//
//    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
//    Basic Linear Algebra Subprograms for Fortran Usage,
//    Algorithm 539,
//    ACM Transactions on Mathematical Software,
//    Volume 5, Number 3, September 1979, pages 308-323.
//
//  Parameters:
//
//    Input, int N, the number of entries in the vector.
//
//    Input, double X[*], the vector whose norm is to be computed.
//
//    Input, int INCX, the increment between successive entries of X.
//
//    Output, double DNRM2, the Euclidean norm of X.
//
{
  double absxi;
  int i;
  int ix;
  double norm;
  double scale;
  double ssq;
  double value;

  if ( n < 1 || incx < 1 )
  {
    norm = 0.0;
  }
  else if ( n == 1 )
  {
    norm = r8_abs ( x[0] );
  }
  else
  {
    scale = 0.0;
    ssq = 1.0;
    ix = 0;

    for ( i = 0; i < n; i++ )
    {
      if ( x[ix] != 0.0 )
      {
        absxi = r8_abs ( x[ix] );
        if ( scale < absxi )
        {
          ssq = 1.0 + ssq * ( scale / absxi ) * ( scale / absxi );
          scale = absxi;
        }
        else
        {
          ssq = ssq + ( absxi / scale ) * ( absxi / scale );
        }
      }
      ix = ix + incx;
    }

    norm  = scale * sqrt ( ssq );
  }

  return norm;
}
//****************************************************************************80

void drot ( int n, double x[], int incx, double y[], int incy, double c,
  double s )

//****************************************************************************80
//
//  Purpose:
//
//    DROT applies a plane rotation.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    02 May 2005
//
//  Author:
//
//    Original FORTRAN77 version by Charles Lawson, Richard Hanson, 
//    David Kincaid, Fred Krogh.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jack Dongarra, Jim Bunch, Cleve Moler, Pete Stewart,
//    LINPACK User's Guide,
//    SIAM, 1979,
//    ISBN13: 978-0-898711-72-1,
//    LC: QA214.L56.
//
//    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
//    Basic Linear Algebra Subprograms for Fortran Usage,
//    Algorithm 539,
//    ACM Transactions on Mathematical Software,
//    Volume 5, Number 3, September 1979, pages 308-323.
//
//  Parameters:
//
//    Input, int N, the number of entries in the vectors.
//
//    Input/output, double X[*], one of the vectors to be rotated.
//
//    Input, int INCX, the increment between successive entries of X.
//
//    Input/output, double Y[*], one of the vectors to be rotated.
//
//    Input, int INCY, the increment between successive elements of Y.
//
//    Input, double C, S, parameters (presumably the cosine and
//    sine of some angle) that define a plane rotation.
//
{
  int i;
  int ix;
  int iy;
  double stemp;

  if ( n <= 0 )
  {
  }
  else if ( incx == 1 && incy == 1 )
  {
    for ( i = 0; i < n; i++ )
    {
      stemp = c * x[i] + s * y[i];
      y[i]  = c * y[i] - s * x[i];
      x[i]  = stemp;
    }
  }
  else
  {
    if ( 0 <= incx )
    {
      ix = 0;
    }
    else
    {
      ix = ( - n + 1 ) * incx;
    }

    if ( 0 <= incy )
    {
      iy = 0;
    }
    else
    {
      iy = ( - n + 1 ) * incy;
    }

    for ( i = 0; i < n; i++ )
    {
      stemp = c * x[ix] + s * y[iy];
      y[iy] = c * y[iy] - s * x[ix];
      x[ix] = stemp;
      ix = ix + incx;
      iy = iy + incy;
    }

  }

  return;
}
//****************************************************************************80

void drotg ( double *sa, double *sb, double *c, double *s )

//****************************************************************************80
//
//  Purpose:
//
//    DROTG constructs a Givens plane rotation.
//
//  Discussion:
//
//    Given values A and B, this routine computes
//
//    SIGMA = sign ( A ) if abs ( A ) >  abs ( B )
//          = sign ( B ) if abs ( A ) <= abs ( B );
//
//    R     = SIGMA * ( A * A + B * B );
//
//    C = A / R if R is not 0
//      = 1     if R is 0;
//
//    S = B / R if R is not 0,
//        0     if R is 0.
//
//    The computed numbers then satisfy the equation
//
//    (  C  S ) ( A ) = ( R )
//    ( -S  C ) ( B ) = ( 0 )
//
//    The routine also computes
//
//    Z = S     if abs ( A ) > abs ( B ),
//      = 1 / C if abs ( A ) <= abs ( B ) and C is not 0,
//      = 1     if C is 0.
//
//    The single value Z encodes C and S, and hence the rotation:
//
//    If Z = 1, set C = 0 and S = 1;
//    If abs ( Z ) < 1, set C = sqrt ( 1 - Z * Z ) and S = Z;
//    if abs ( Z ) > 1, set C = 1/ Z and S = sqrt ( 1 - C * C );
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    15 May 2006
//
//  Author:
//
//    Original FORTRAN77 version by Charles Lawson, Richard Hanson, 
//    David Kincaid, Fred Krogh.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jack Dongarra, Jim Bunch, Cleve Moler, Pete Stewart,
//    LINPACK User's Guide,
//    SIAM, 1979,
//    ISBN13: 978-0-898711-72-1,
//    LC: QA214.L56.
//
//    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
//    Basic Linear Algebra Subprograms for Fortran Usage,
//    Algorithm 539,
//    ACM Transactions on Mathematical Software,
//    Volume 5, Number 3, September 1979, pages 308-323.
//
//  Parameters:
//
//    Input/output, double *SA, *SB,  On input, SA and SB are the values
//    A and B.  On output, SA is overwritten with R, and SB is
//    overwritten with Z.
//
//    Output, double *C, *S, the cosine and sine of the Givens rotation.
//
{
  double r;
  double roe;
  double scale;
  double z;

  if ( r8_abs ( *sb ) < r8_abs ( *sa ) )
  {
    roe = *sa;
  }
  else
  {
    roe = *sb;
  }

  scale = r8_abs ( *sa ) + r8_abs ( *sb );

  if ( scale == 0.0 )
  {
    *c = 1.0;
    *s = 0.0;
    r = 0.0;
  }
  else
  {
    r = scale * sqrt ( ( *sa / scale ) * ( *sa / scale )
                     + ( *sb / scale ) * ( *sb / scale ) );
    r = r8_sign ( roe ) * r;
    *c = *sa / r;
    *s = *sb / r;
  }

  if ( 0.0 < r8_abs ( *c ) && r8_abs ( *c ) <= *s )
  {
    z = 1.0 / *c;
  }
  else
  {
    z = *s;
  }

  *sa = r;
  *sb = z;

  return;
}
//****************************************************************************80

void dscal ( int n, double sa, double x[], int incx )

//****************************************************************************80
//
//  Purpose:
//
//    DSCAL scales a vector by a constant.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    02 May 2005
//
//  Author:
//
//    Original FORTRAN77 version by Charles Lawson, Richard Hanson, 
//    David Kincaid, Fred Krogh.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jack Dongarra, Jim Bunch, Cleve Moler, Pete Stewart,
//    LINPACK User's Guide,
//    SIAM, 1979,
//    ISBN13: 978-0-898711-72-1,
//    LC: QA214.L56.
//
//    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
//    Basic Linear Algebra Subprograms for Fortran Usage,
//    Algorithm 539,
//    ACM Transactions on Mathematical Software,
//    Volume 5, Number 3, September 1979, pages 308-323.
//
//  Parameters:
//
//    Input, int N, the number of entries in the vector.
//
//    Input, double SA, the multiplier.
//
//    Input/output, double X[*], the vector to be scaled.
//
//    Input, int INCX, the increment between successive entries of X.
//
{
  int i;
  int ix;
  int m;

  if ( n <= 0 )
  {
  }
  else if ( incx == 1 )
  {
    m = n % 5;

    for ( i = 0; i < m; i++ )
    {
      x[i] = sa * x[i];
    }

    for ( i = m; i < n; i = i + 5 )
    {
      x[i]   = sa * x[i];
      x[i+1] = sa * x[i+1];
      x[i+2] = sa * x[i+2];
      x[i+3] = sa * x[i+3];
      x[i+4] = sa * x[i+4];
    }
  }
  else
  {
    if ( 0 <= incx )
    {
      ix = 0;
    }
    else
    {
      ix = ( - n + 1 ) * incx;
    }

    for ( i = 0; i < n; i++ )
    {
      x[ix] = sa * x[ix];
      ix = ix + incx;
    }

  }

  return;
}
//****************************************************************************80

int dsvdc ( double a[], int lda, int m, int n, double s[], double e[], 
  double u[], int ldu, double v[], int ldv, double work[], int job )

//****************************************************************************80
//
//  Purpose:
//
//    DSVDC computes the singular value decomposition of a real rectangular matrix.
//
//  Discussion:
//
//    This routine reduces an M by N matrix A to diagonal form by orthogonal
//    transformations U and V.  The diagonal elements S(I) are the singular
//    values of A.  The columns of U are the corresponding left singular
//    vectors, and the columns of V the right singular vectors.
//
//    The form of the singular value decomposition is then
//
//      A(MxN) = U(MxM) * S(MxN) * V(NxN)'
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    03 May 2007
//
//  Author:
//
//    Original FORTRAN77 version by Jack Dongarra, Cleve Moler, Jim Bunch, 
//    Pete Stewart.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jack Dongarra, Cleve Moler, Jim Bunch, Pete Stewart,
//    LINPACK User's Guide,
//    SIAM, (Society for Industrial and Applied Mathematics),
//    3600 University City Science Center,
//    Philadelphia, PA, 19104-2688.
//    ISBN 0-89871-172-X
//
//  Parameters:
//
//    Input/output, double A[LDA*N].  On input, the M by N matrix whose
//    singular value decomposition is to be computed.  On output, the matrix
//    has been destroyed.  Depending on the user's requests, the matrix may 
//    contain other useful information.
//
//    Input, int LDA, the leading dimension of the array A.
//    LDA must be at least M.
//
//    Input, int M, the number of rows of the matrix.
//
//    Input, int N, the number of columns of the matrix A.
//
//    Output, double S[MM], where MM = min(M+1,N).  The first
//    min(M,N) entries of S contain the singular values of A arranged in
//    descending order of magnitude.
//
//    Output, double E[MM], where MM = min(M+1,N), ordinarily contains zeros.
//    However see the discussion of INFO for exceptions.
//
//    Output, double U[LDU*K].  If JOBA = 1 then K = M;
//    if 2 <= JOBA, then K = min(M,N).  U contains the M by M matrix of left singular
//    vectors.  U is not referenced if JOBA = 0.  If M <= N or if JOBA = 2, then
//    U may be identified with A in the subroutine call.
//
//    Input, int LDU, the leading dimension of the array U.
//    LDU must be at least M.
//
//    Output, double V[LDV*N], the N by N matrix of right singular vectors.
//    V is not referenced if JOB is 0.  If N <= M, then V may be identified
//    with A in the subroutine call.
//
//    Input, int LDV, the leading dimension of the array V.
//    LDV must be at least N.
//
//    Workspace, double WORK[M].
//
//    Input, int JOB, controls the computation of the singular
//    vectors.  It has the decimal expansion AB with the following meaning:
//      A =  0, do not compute the left singular vectors.
//      A =  1, return the M left singular vectors in U.
//      A >= 2, return the first min(M,N) singular vectors in U.
//      B =  0, do not compute the right singular vectors.
//      B =  1, return the right singular vectors in V.
//
//    Output, int *DSVDC, status indicator INFO.
//    The singular values (and their corresponding singular vectors)
//    S(*INFO+1), S(*INFO+2),...,S(MN) are correct.  Here MN = min ( M, N ).
//    Thus if *INFO is 0, all the singular values and their vectors are
//    correct.  In any event, the matrix B = U' * A * V is the bidiagonal
//    matrix with the elements of S on its diagonal and the elements of E on
//    its superdiagonal.  Thus the singular values of A and B are the same.
//
{
  double b;
  double c;
  double cs;
  double el;
  double emm1;
  double f;
  double g;
  int i;
  int info;
  int iter;
  int j;
  int jobu;
  int k;
  int kase;
  int kk;
  int l;
  int ll;
  int lls;
  int ls;
  int lu;
  int maxit = 30;
  int mm;
  int mm1;
  int mn;
  int mp1;
  int nct;
  int nctp1;
  int ncu;
  int nrt;
  int nrtp1;
  double scale;
  double shift;
  double sl;
  double sm;
  double smm1;
  double sn;
  double t;
  double t1;
  double test;
  bool wantu;
  bool wantv;
  double ztest;
//
//  Determine what is to be computed.
//
  info = 0;
  wantu = false;
  wantv = false;
  jobu = ( job % 100 ) / 10;

  if ( 1 < jobu )
  {
    ncu = i4_min ( m, n );
  }
  else
  {
    ncu = m;
  }

  if ( jobu != 0 )
  {
    wantu = true;
  }

  if ( ( job % 10 ) != 0 )
  {
    wantv = true;
  }
//
//  Reduce A to bidiagonal form, storing the diagonal elements
//  in S and the super-diagonal elements in E.
//
  nct = i4_min ( m-1, n );
  nrt = i4_max ( 0, i4_min ( m, n-2 ) );
  lu = i4_max ( nct, nrt );

  for ( l = 1; l <= lu; l++ )
  {
//
//  Compute the transformation for the L-th column and
//  place the L-th diagonal in S(L).
//
    if ( l <= nct )
    {
      s[l-1] = dnrm2 ( m-l+1, a+l-1+(l-1)*lda, 1 );

      if ( s[l-1] != 0.0 )
      {
        if ( a[l-1+(l-1)*lda] != 0.0 )
        {
          s[l-1] = r8_sign ( a[l-1+(l-1)*lda] ) * r8_abs ( s[l-1] );
        }
        dscal ( m-l+1, 1.0 / s[l-1], a+l-1+(l-1)*lda, 1 );
        a[l-1+(l-1)*lda] = 1.0 + a[l-1+(l-1)*lda];
      }
      s[l-1] = -s[l-1];
    }

    for ( j = l+1; j <= n; j++ )
    {
//
//  Apply the transformation.
//
      if ( l <= nct && s[l-1] != 0.0 )
      {
        t = - ddot ( m-l+1, a+l-1+(l-1)*lda, 1, a+l-1+(j-1)*lda, 1 ) 
          / a[l-1+(l-1)*lda];
        daxpy ( m-l+1, t, a+l-1+(l-1)*lda, 1, a+l-1+(j-1)*lda, 1 );
      }
//
//  Place the L-th row of A into E for the
//  subsequent calculation of the row transformation.
//
      e[j-1] = a[l-1+(j-1)*lda];
    }
//
//  Place the transformation in U for subsequent back multiplication.
//
    if ( wantu && l <= nct )
    {
      for ( i = l; i <= m; i++ )
      {
        u[i-1+(l-1)*ldu] = a[i-1+(l-1)*lda];
      }
    }

    if ( l <= nrt )
    {
//
//  Compute the L-th row transformation and place the
//  L-th superdiagonal in E(L).
//
      e[l-1] = dnrm2 ( n-l, e+l, 1 );

      if ( e[l-1] != 0.0 )
      {
        if ( e[l] != 0.0 )
        {
          e[l-1] = r8_sign ( e[l] ) * r8_abs ( e[l-1] );
        }
        dscal ( n-l, 1.0 / e[l-1], e+l, 1 );
        e[l] = 1.0 + e[l];
      }

      e[l-1] = -e[l-1];
//
//  Apply the transformation.
//
      if ( l+1 <= m && e[l-1] != 0.0 )
      {
        for ( j = l+1; j <= m; j++ )
        {
          work[j-1] = 0.0;
        }

        for ( j = l+1; j <= n; j++ )
        {
          daxpy ( m-l, e[j-1], a+l+(j-1)*lda, 1, work+l, 1 );
        }

        for ( j = l+1; j <= n; j++ )
        {
          daxpy ( m-l, -e[j-1]/e[l], work+l, 1, a+l+(j-1)*lda, 1 );
        }
      }
//
//  Place the transformation in V for subsequent back multiplication.
//
      if ( wantv )
      {
        for ( j = l+1; j <= n; j++ )
        {
          v[j-1+(l-1)*ldv] = e[j-1];
        }
      }
    }
  }
//
//  Set up the final bidiagonal matrix of order MN.
//
  mn = i4_min ( m + 1, n );
  nctp1 = nct + 1;
  nrtp1 = nrt + 1;

  if ( nct < n )
  {
    s[nctp1-1] = a[nctp1-1+(nctp1-1)*lda];
  }

  if ( m < mn )
  {
    s[mn-1] = 0.0;
  }

  if ( nrtp1 < mn )
  {
    e[nrtp1-1] = a[nrtp1-1+(mn-1)*lda];
  }

  e[mn-1] = 0.0;
//
//  If required, generate U.
//
  if ( wantu )
  {
    for ( i = 1; i <= m; i++ )
    {
      for ( j = nctp1; j <= ncu; j++ )
      {
        u[(i-1)+(j-1)*ldu] = 0.0;
      }
    }

    for ( j = nctp1; j <= ncu; j++ )
    {
      u[j-1+(j-1)*ldu] = 1.0;
    }

    for ( ll = 1; ll <= nct; ll++ )
    {
      l = nct - ll + 1;

      if ( s[l-1] != 0.0 )
      {
        for ( j = l+1; j <= ncu; j++ )
        {
          t = - ddot ( m-l+1, u+(l-1)+(l-1)*ldu, 1, u+(l-1)+(j-1)*ldu, 1 ) 
            / u[l-1+(l-1)*ldu];
          daxpy ( m-l+1, t, u+(l-1)+(l-1)*ldu, 1, u+(l-1)+(j-1)*ldu, 1 );
        }

        dscal ( m-l+1, -1.0, u+(l-1)+(l-1)*ldu, 1 );
        u[l-1+(l-1)*ldu] = 1.0 + u[l-1+(l-1)*ldu];
        for ( i = 1; i <= l-1; i++ )
        {
          u[i-1+(l-1)*ldu] = 0.0;
        }
      }
      else
      {
        for ( i = 1; i <= m; i++ )
        {
          u[i-1+(l-1)*ldu] = 0.0;
        }
        u[l-1+(l-1)*ldu] = 1.0;
      }
    }
  }
//
//  If it is required, generate V.
//
  if ( wantv )
  {
    for ( ll = 1; ll <= n; ll++ )
    {
      l = n - ll + 1;

      if ( l <= nrt && e[l-1] != 0.0 )
      {
        for ( j = l+1; j <= n; j++ )
        {
          t = - ddot ( n-l, v+l+(l-1)*ldv, 1, v+l+(j-1)*ldv, 1 ) 
            / v[l+(l-1)*ldv];
          daxpy ( n-l, t, v+l+(l-1)*ldv, 1, v+l+(j-1)*ldv, 1 );
        }

      }
      for ( i = 1; i <= n; i++ )
      {
        v[i-1+(l-1)*ldv] = 0.0;
      }
      v[l-1+(l-1)*ldv] = 1.0;
    }
  }
//
//  Main iteration loop for the singular values.
//
  mm = mn;
  iter = 0;

  while ( 0 < mn )
  {
//
//  If too many iterations have been performed, set flag and return.
//
    if ( maxit <= iter )
    {
      info = mn;
      return info;
    }
//
//  This section of the program inspects for
//  negligible elements in the S and E arrays.
//
//  On completion the variables KASE and L are set as follows:
//
//  KASE = 1     if S(MN) and E(L-1) are negligible and L < MN
//  KASE = 2     if S(L) is negligible and L < MN
//  KASE = 3     if E(L-1) is negligible, L < MN, and
//               S(L), ..., S(MN) are not negligible (QR step).
//  KASE = 4     if E(MN-1) is negligible (convergence).
//
    for ( ll = 1; ll <= mn; ll++ )
    {
      l = mn - ll;

      if ( l == 0 )
      {
        break;
      }

      test = r8_abs ( s[l-1] ) + r8_abs ( s[l] );
      ztest = test + r8_abs ( e[l-1] );

      if ( ztest == test )
      {
        e[l-1] = 0.0;
        break;
      }
    }

    if ( l == mn - 1 )
    {
      kase = 4;
    }
    else
    {
      mp1 = mn + 1;

      for ( lls = l+1; lls <= mn+1; lls++ )
      {
        ls = mn - lls + l + 1;

        if ( ls == l )
        {
          break;
        }

        test = 0.0;
        if ( ls != mn )
        {
          test = test + r8_abs ( e[ls-1] );
        }

        if ( ls != l + 1 )
        {
          test = test + r8_abs ( e[ls-2] );
        }

        ztest = test + r8_abs ( s[ls-1] );

        if ( ztest == test )
        {
          s[ls-1] = 0.0;
          break;
        }

      }

      if ( ls == l )
      {
        kase = 3;
      }
      else if ( ls == mn )
      {
        kase = 1;
      }
      else
      {
        kase = 2;
        l = ls;
      }
    }

    l = l + 1;
//
//  Deflate negligible S(MN).
//
    if ( kase == 1 )
    {
      mm1 = mn - 1;
      f = e[mn-2];
      e[mn-2] = 0.0;

      for ( kk = 1; kk <= mm1; kk++ )
      {
        k = mm1 - kk + l;
        t1 = s[k-1];
        drotg ( &t1, &f, &cs, &sn );
        s[k-1] = t1;

        if ( k != l )
        {
          f = -sn * e[k-2];
          e[k-2] = cs * e[k-2];
        }

        if ( wantv )
        {
          drot ( n, v+0+(k-1)*ldv, 1, v+0+(mn-1)*ldv, 1, cs, sn );
        }
      }
    }
//
//  Split at negligible S(L).
//
    else if ( kase == 2 )
    {
      f = e[l-2];
      e[l-2] = 0.0;

      for ( k = l; k <= mn; k++ )
      {
        t1 = s[k-1];
        drotg ( &t1, &f, &cs, &sn );
        s[k-1] = t1;
        f = - sn * e[k-1];
        e[k-1] = cs * e[k-1];
        if ( wantu )
        {
          drot ( m, u+0+(k-1)*ldu, 1, u+0+(l-2)*ldu, 1, cs, sn );
        }
      }
    }
//
//  Perform one QR step.
//
    else if ( kase == 3 )
    {
//
//  Calculate the shift.
//
      scale = r8_max ( r8_abs ( s[mn-1] ), 
              r8_max ( r8_abs ( s[mn-2] ), 
              r8_max ( r8_abs ( e[mn-2] ), 
              r8_max ( r8_abs ( s[l-1] ), r8_abs ( e[l-1] ) ) ) ) );

      sm = s[mn-1] / scale;
      smm1 = s[mn-2] / scale;
      emm1 = e[mn-2] / scale;
      sl = s[l-1] / scale;
      el = e[l-1] / scale;
      b = ( ( smm1 + sm ) * ( smm1 - sm ) + emm1 * emm1 ) / 2.0;
      c = ( sm * emm1 ) * ( sm * emm1 );
      shift = 0.0;

      if ( b != 0.0 || c != 0.0 )
      {
        shift = sqrt ( b * b + c );
        if ( b < 0.0 )
        {
          shift = -shift;
        }
        shift = c / ( b + shift );
      }

      f = ( sl + sm ) * ( sl - sm ) - shift;
      g = sl * el;
//
//  Chase zeros.
//
      mm1 = mn - 1;

      for ( k = l; k <= mm1; k++ )
      {
        drotg ( &f, &g, &cs, &sn );

        if ( k != l )
        {
          e[k-2] = f;
        }

        f = cs * s[k-1] + sn * e[k-1];
        e[k-1] = cs * e[k-1] - sn * s[k-1];
        g = sn * s[k];
        s[k] = cs * s[k];

        if ( wantv )
        {
          drot ( n, v+0+(k-1)*ldv, 1, v+0+k*ldv, 1, cs, sn );
        }

        drotg ( &f, &g, &cs, &sn );
        s[k-1] = f;
        f = cs * e[k-1] + sn * s[k];
        s[k] = -sn * e[k-1] + cs * s[k];
        g = sn * e[k];
        e[k] = cs * e[k];

        if ( wantu && k < m )
        {
          drot ( m, u+0+(k-1)*ldu, 1, u+0+k*ldu, 1, cs, sn );
        }
      }
      e[mn-2] = f;
      iter = iter + 1;
    }
//
//  Convergence.
//
    else if ( kase == 4 )
    {
//
//  Make the singular value nonnegative.
//
      if ( s[l-1] < 0.0 )
      {
        s[l-1] = -s[l-1];
        if ( wantv )
        {
          dscal ( n, -1.0, v+0+(l-1)*ldv, 1 );
        }
      }
//
//  Order the singular value.
//
      for ( ; ; )
      {
        if ( l == mm )
        {
          break;
        }

        if ( s[l] <= s[l-1] )
        {
          break;
        }

        t = s[l-1];
        s[l-1] = s[l];
        s[l] = t;

        if ( wantv && l < n )
        {
          dswap ( n, v+0+(l-1)*ldv, 1, v+0+l*ldv, 1 );
        }

        if ( wantu && l < m )
        {
          dswap ( m, u+0+(l-1)*ldu, 1, u+0+l*ldu, 1 );
        }

        l = l + 1;
      }
      iter = 0;
      mn = mn - 1;
    }
  }

  return info;
}
//****************************************************************************80

void dswap ( int n, double x[], int incx, double y[], int incy )

//****************************************************************************80
//
//  Purpose:
//
//    DSWAP interchanges two vectors.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    02 May 2005
//
//  Author:
//
//    Original FORTRAN77 version by Charles Lawson, Richard Hanson, 
//    David Kincaid, Fred Krogh.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Jack Dongarra, Jim Bunch, Cleve Moler, Pete Stewart,
//    LINPACK User's Guide,
//    SIAM, 1979,
//    ISBN13: 978-0-898711-72-1,
//    LC: QA214.L56.
//
//    Charles Lawson, Richard Hanson, David Kincaid, Fred Krogh,
//    Basic Linear Algebra Subprograms for Fortran Usage,
//    Algorithm 539,
//    ACM Transactions on Mathematical Software,
//    Volume 5, Number 3, September 1979, pages 308-323.
//
//  Parameters:
//
//    Input, int N, the number of entries in the vectors.
//
//    Input/output, double X[*], one of the vectors to swap.
//
//    Input, int INCX, the increment between successive entries of X.
//
//    Input/output, double Y[*], one of the vectors to swap.
//
//    Input, int INCY, the increment between successive elements of Y.
//
{
  int i;
  int ix;
  int iy;
  int m;
  double temp;

  if ( n <= 0 )
  {
  }
  else if ( incx == 1 && incy == 1 )
  {
    m = n % 3;

    for ( i = 0; i < m; i++ )
    {
      temp = x[i];
      x[i] = y[i];
      y[i] = temp;
    }

    for ( i = m; i < n; i = i + 3 )
    {
      temp = x[i];
      x[i] = y[i];
      y[i] = temp;

      temp = x[i+1];
      x[i+1] = y[i+1];
      y[i+1] = temp;

      temp = x[i+2];
      x[i+2] = y[i+2];
      y[i+2] = temp;
    }
  }
  else
  {
    if ( 0 <= incx )
    {
      ix = 0;
    }
    else
    {
      ix = ( - n + 1 ) * incx;
    }

    if ( 0 <= incy )
    {
      iy = 0;
    }
    else
    {
      iy = ( - n + 1 ) * incy;
    }

    for ( i = 0; i < n; i++ )
    {
      temp = x[ix];
      x[ix] = y[iy];
      y[iy] = temp;
      ix = ix + incx;
      iy = iy + incy;
    }

  }

  return;
}
//****************************************************************************80

int i4_max ( int i1, int i2 )

//****************************************************************************80
//
//  Purpose:
//
//    I4_MAX returns the maximum of two I4's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    13 October 1998
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int I1, I2, are two integers to be compared.
//
//    Output, int I4_MAX, the larger of I1 and I2.
//
{
  int value;

  if ( i2 < i1 )
  {
    value = i1;
  }
  else
  {
    value = i2;
  }
  return value;
}
//****************************************************************************80

int i4_min ( int i1, int i2 )

//****************************************************************************80
//
//  Purpose:
//
//    I4_MIN returns the minimum of two I4's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    13 October 1998
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int I1, I2, two integers to be compared.
//
//    Output, int I4_MIN, the smaller of I1 and I2.
//
{
  int value;

  if ( i1 < i2 )
  {
    value = i1;
  }
  else
  {
    value = i2;
  }
  return value;
}
//****************************************************************************80

int i4_power ( int i, int j )

//****************************************************************************80
//
//  Purpose:
//
//    I4_POWER returns the value of I^J.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 April 2004
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int I, J, the base and the power.  J should be nonnegative.
//
//    Output, int I4_POWER, the value of I^J.
//
{
  int k;
  int value;

  if ( j < 0 )
  {
    if ( i == 1 )
    {
      value = 1;
    }
    else if ( i == 0 )
    {
      cerr << "\n";
      cerr << "I4_POWER - Fatal error!\n";
      cerr << "  I^J requested, with I = 0 and J negative.\n";
      exit ( 1 );
    }
    else
    {
      value = 0;
    }
  }
  else if ( j == 0 )
  {
    if ( i == 0 )
    {
      cerr << "\n";
      cerr << "I4_POWER - Fatal error!\n";
      cerr << "  I^J requested, with I = 0 and J = 0.\n";
      exit ( 1 );
    }
    else
    {
      value = 1;
    }
  }
  else if ( j == 1 )
  {
    value = i;
  }
  else
  {
    value = 1;
    for ( k = 1; k <= j; k++ )
    {
      value = value * i;
    }
  }
  return value;
}
//****************************************************************************80

void phi1 ( int n, double r[], double r0, double v[] )

//****************************************************************************80
//
//  Purpose:
//
//    PHI1 evaluates the multiquadric radial basis function.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    30 June 2012
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    William Press, Brian Flannery, Saul Teukolsky, William Vetterling,
//    Numerical Recipes in FORTRAN: The Art of Scientific Computing,
//    Third Edition,
//    Cambridge University Press, 2007,
//    ISBN13: 978-0-521-88068-8,
//    LC: QA297.N866.
//
//  Parameters:
//
//    Input, int N, the number of points.
//
//    Input, double R[N], the radial separation.
//    0 < R.
//
//    Input, double R0, a scale factor.
//
//    Output, double V[N], the value of the radial basis function.
//
{
  int i;

  for ( i = 0; i < n; i++ )
  {
    v[i] = sqrt ( r[i] * r[i] + r0 * r0 );
  }
  return;
}
//****************************************************************************80

void phi2 ( int n, double r[], double r0, double v[] )

//****************************************************************************80
//
//  Purpose:
//
//    PHI2 evaluates the inverse multiquadric radial basis function.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    30 June 2012
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    William Press, Brian Flannery, Saul Teukolsky, William Vetterling,
//    Numerical Recipes in FORTRAN: The Art of Scientific Computing,
//    Third Edition,
//    Cambridge University Press, 2007,
//    ISBN13: 978-0-521-88068-8,
//    LC: QA297.N866.
//
//  Parameters:
//
//    Input, int N, the number of points.
//
//    Input, double R[N], the radial separation.
//    0 < R.
//
//    Input, double R0, a scale factor.
//
//    Output, double V[N], the value of the radial basis function.
//
{
  int i;

  for ( i = 0; i < n; i++ )
  {
    v[i] = 1.0 / sqrt ( r[i] * r[i] + r0 * r0 );
  }
  return;
}
//****************************************************************************80

void phi3 ( int n, double r[], double r0, double v[] )

//****************************************************************************80
//
//  Purpose:
//
//    PHI3 evaluates the thin-plate spline radial basis function.
//
//  Discussion:
//
//    Note that PHI3(R,R0) is negative if R < R0.  Thus, for this basis function,
//    it may be desirable to choose a value of R0 smaller than any possible R.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    30 June 2012
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    William Press, Brian Flannery, Saul Teukolsky, William Vetterling,
//    Numerical Recipes in FORTRAN: The Art of Scientific Computing,
//    Third Edition,
//    Cambridge University Press, 2007,
//    ISBN13: 978-0-521-88068-8,
//    LC: QA297.N866.
//
//  Parameters:
//
//    Input, int N, the number of points.
//
//    Input, double R[N], the radial separation.
//    0 < R.
//
//    Input, double R0, a scale factor.
//
//    Output, double V[N], the value of the radial basis function.
//
{
  int i;

  for ( i = 0; i < n; i++ )
  {
    if ( r[i] <= 0.0 )
    {
      v[i] = 0.0;
    }
    else
    {
      v[i] = r[i] * r[i] * log ( r[i] / r0 );
    }
  }
  return;
}
//****************************************************************************80

void phi4 ( int n, double r[], double r0, double v[] )

//****************************************************************************80
//
//  Purpose:
//
//    PHI4 evaluates the gaussian radial basis function.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    30 June 2012
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    William Press, Brian Flannery, Saul Teukolsky, William Vetterling,
//    Numerical Recipes in FORTRAN: The Art of Scientific Computing,
//    Third Edition,
//    Cambridge University Press, 2007,
//    ISBN13: 978-0-521-88068-8,
//    LC: QA297.N866.
//
//  Parameters:
//
//    Input, int N, the number of points.
//
//    Input, double R[N], the radial separation.
//    0 < R.
//
//    Input, double R0, a scale factor.
//
//    Output, double V[N], the value of the radial basis function.
//
{
  int i;

  for ( i = 0; i < n; i++ )
  {
    v[i] = exp ( - 0.5 * r[i] * r[i] / r0 / r0 );
  }
  return;
}
//****************************************************************************80

double r8_abs ( double x )

//****************************************************************************80
//
//  Purpose:
//
//    R8_ABS returns the absolute value of an R8.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    14 November 2006
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, double X, the quantity whose absolute value is desired.
//
//    Output, double R8_ABS, the absolute value of X.
//
{
  double value;

  if ( 0.0 <= x )
  {
    value = + x;
  }
  else
  {
    value = - x;
  }
  return value;
}
//****************************************************************************80

double r8_max ( double x, double y )

//****************************************************************************80
//
//  Purpose:
//
//    R8_MAX returns the maximum of two R8's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    18 August 2004
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, double X, Y, the quantities to compare.
//
//    Output, double R8_MAX, the maximum of X and Y.
//
{
  double value;

  if ( y < x )
  {
    value = x;
  }
  else
  {
    value = y;
  }
  return value;
}
//****************************************************************************80

double r8_sign ( double x )

//****************************************************************************80
//
//  Purpose:
//
//    R8_SIGN returns the sign of an R8.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    18 October 2004
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, double X, the number whose sign is desired.
//
//    Output, double R8_SIGN, the sign of X.
//
{
  double value;

  if ( x < 0.0 )
  {
    value = -1.0;
  }
  else
  {
    value = 1.0;
  }
  return value;
}
//****************************************************************************80

double *r8mat_copy_new ( int m, int n, double a1[] )

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_COPY_NEW copies one R8MAT to a "new" R8MAT.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8's, which
//    may be stored as a vector in column-major order.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    03 July 2008
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, N, the number of rows and columns.
//
//    Input, double A1[M*N], the matrix to be copied.
//
//    Output, double R8MAT_COPY_NEW[M*N], the copy of A1.
//
{
  double *a2;
  int i;
  int j;

  a2 = new double[m*n];

  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      a2[i+j*m] = a1[i+j*m];
    }
  }
  return a2;
}
//****************************************************************************80

double *r8mat_mv_new ( int m, int n, double a[], double x[] )

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_MV_NEW multiplies a matrix times a vector.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//    For this routine, the result is returned as the function value.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    11 April 2007
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, N, the number of rows and columns of the matrix.
//
//    Input, double A[M,N], the M by N matrix.
//
//    Input, double X[N], the vector to be multiplied by A.
//
//    Output, double R8MAT_MV_NEW[M], the product A*X.
//
{
  int i;
  int j;
  double *y;

  y = new double[m];

  for ( i = 0; i < m; i++ )
  {
    y[i] = 0.0;
    for ( j = 0; j < n; j++ )
    {
      y[i] = y[i] + a[i+j*m] * x[j];
    }
  }

  return y;
}
//****************************************************************************80

double *r8mat_solve_svd ( int m, int n, double a[], double b[] )

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_SOLVE_SVD solves a linear system A*x=b using the SVD.
//
//  Discussion:
//
//    When the system is determined, the solution is the solution in the
//    ordinary sense, and A*x = b.
//
//    When the system is overdetermined, the solution minimizes the
//    L2 norm of the residual ||A*x-b||.
//
//    When the system is underdetermined, ||A*x-b|| should be zero, and
//    the solution is the solution of minimum L2 norm, ||x||.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    30 June 2012
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, N, the number of rows and columns
//    in the matrix A.
//
//    Input, double A[M,*N], the matrix.
//
//    Input, double B[M], the right hand side.
//
//    Output, double R8MAT_SOLVE_SVD[N], the solution.
//
{
  double *a_copy;
  double *a_pseudo;
  double *e;
  int i;
  int info;
  int j;
  int k;
  int l;
  int lda;
  int ldu;
  int ldv;
  int job;
  int lwork;
  double *s;
  double *sp;
  double *sdiag;
  double *u;
  double *v;
  double *work;
  double *x;
//
//  Compute the SVD decomposition.
//
  a_copy = r8mat_copy_new ( m, n, a );
  lda = m;
  sdiag = new double[ i4_max ( m + 1, n ) ];
  e = new double[ i4_max ( m + 1, n ) ];
  u = new double[ m * m ];
  ldu = m;
  v = new double[ n * n ];
  ldv = n;
  work = new double[ m ];
  job = 11;

  info = dsvdc ( a_copy, lda, m, n, sdiag, e, u, ldu, v, ldv, work, job );

  if ( info != 0 )
  {
    cerr << "\n";
    cerr << "R8MAT_SOLVE_SVD - Fatal error!\n";
    cerr << "  The SVD could not be calculated.\n";
    cerr << "  LINPACK routine DSVDC returned a nonzero\n";
    cerr << "  value of the error flag, INFO = " << info << "\n";
    exit ( 1 );
  }

  s = new double [ m * n ];

  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      s[i+j*m] = 0.0;
    }
  }
  for ( i = 0; i < i4_min ( m, n ); i++ )
  {
    s[i+i*m] = sdiag[i];
  }
//
//  Compute the pseudo inverse.
//
  sp = new double [ n * m ];

  for ( j = 0; j < m; j++ )
  {
    for ( i = 0; i < n; i++ )
    {
      sp[i+j*m] = 0.0;
    }
  }
  for ( i = 0; i < i4_min ( m, n ); i++ )
  {
    if ( s[i+i*m] != 0.0 )
    {
      sp[i+i*n] = 1.0 / s[i+i*m];
    }
  }

  a_pseudo = new double[ n * m ];

  for ( j = 0; j < m; j++ )
  {
    for ( i = 0; i < n; i++ )
    {
      a_pseudo[i+j*n] = 0.0;
      for ( k = 0; k < n; k++ )
      {
        for ( l = 0; l < m; l++ )
        {
          a_pseudo[i+j*n] = a_pseudo[i+j*n] + v[i+k*n] * sp[k+l*n] * u[j+l*m];
        }
      }
    }
  }
//
//  Compute x = A_pseudo * b.
//
  x = r8mat_mv_new ( n, m, a_pseudo, b );

  delete [] a_copy;
  delete [] a_pseudo;
  delete [] e;
  delete [] s;
  delete [] sdiag;
  delete [] sp;
  delete [] u;
  delete [] v;
  delete [] work;

  return x;
}
//****************************************************************************80

void r8mat_transpose_print ( int m, int n, double a[], string title )

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_TRANSPOSE_PRINT prints an R8MAT, transposed.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    10 September 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, N, the number of rows and columns.
//
//    Input, double A[M*N], an M by N matrix to be printed.
//
//    Input, string TITLE, a title.
//
{
  r8mat_transpose_print_some ( m, n, a, 1, 1, m, n, title );

  return;
}
//****************************************************************************80

void r8mat_transpose_print_some ( int m, int n, double a[], int ilo, int jlo,
  int ihi, int jhi, string title )

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_TRANSPOSE_PRINT_SOME prints some of an R8MAT, transposed.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    20 August 2010
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, N, the number of rows and columns.
//
//    Input, double A[M*N], an M by N matrix to be printed.
//
//    Input, int ILO, JLO, the first row and column to print.
//
//    Input, int IHI, JHI, the last row and column to print.
//
//    Input, string TITLE, a title.
//
{
# define INCX 5

  int i;
  int i2;
  int i2hi;
  int i2lo;
  int inc;
  int j;
  int j2hi;
  int j2lo;

  cout << "\n";
  cout << title << "\n";

  if ( m <= 0 || n <= 0 )
  {
    cout << "\n";
    cout << "  (None)\n";
    return;
  }

  for ( i2lo = i4_max ( ilo, 1 ); i2lo <= i4_min ( ihi, m ); i2lo = i2lo + INCX )
  {
    i2hi = i2lo + INCX - 1;
    i2hi = i4_min ( i2hi, m );
    i2hi = i4_min ( i2hi, ihi );

    inc = i2hi + 1 - i2lo;

    cout << "\n";
    cout << "  Row: ";
    for ( i = i2lo; i <= i2hi; i++ )
    {
      cout << setw(7) << i - 1 << "       ";
    }
    cout << "\n";
    cout << "  Col\n";
    cout << "\n";

    j2lo = i4_max ( jlo, 1 );
    j2hi = i4_min ( jhi, n );

    for ( j = j2lo; j <= j2hi; j++ )
    {
      cout << setw(5) << j - 1 << ":";
      for ( i2 = 1; i2 <= inc; i2++ )
      {
        i = i2lo - 1 + i2;
        cout << setw(14) << a[(i-1)+(j-1)*m];
      }
      cout << "\n";
    }
  }

  return;
# undef INCX
}
//****************************************************************************80

double *r8mat_uniform_new ( int m, int n, double a, double b, int &seed )

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_UNIFORM_NEW returns a scaled pseudorandom R8MAT.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8's,  stored as a vector
//    in column-major order.
//
//    This routine implements the recursion
//
//      seed = 16807 * seed mod ( 2^31 - 1 )
//      unif = seed / ( 2^31 - 1 )
//
//    The integer arithmetic never requires more than 32 bits,
//    including a sign bit.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    03 October 2005
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    Paul Bratley, Bennett Fox, Linus Schrage,
//    A Guide to Simulation,
//    Springer Verlag, pages 201-202, 1983.
//
//    Bennett Fox,
//    Algorithm 647:
//    Implementation and Relative Efficiency of Quasirandom
//    Sequence Generators,
//    ACM Transactions on Mathematical Software,
//    Volume 12, Number 4, pages 362-376, 1986.
//
//    Philip Lewis, Allen Goodman, James Miller,
//    A Pseudo-Random Number Generator for the System/360,
//    IBM Systems Journal,
//    Volume 8, pages 136-143, 1969.
//
//  Parameters:
//
//    Input, int M, N, the number of rows and columns.
//
//    Input, double A, B, the limits of the pseudorandom values.
//
//    Input/output, int &SEED, the "seed" value.  Normally, this
//    value should not be 0, otherwise the output value of SEED
//    will still be 0, and R8_UNIFORM will be 0.  On output, SEED has
//    been updated.
//
//    Output, double R8MAT_UNIFORM_NEW[M*N], a matrix of pseudorandom values.
//
{
  int i;
  int j;
  int k;
  double *r;

  r = new double[m*n];

  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      k = seed / 127773;

      seed = 16807 * ( seed - k * 127773 ) - k * 2836;

      if ( seed < 0 )
      {
        seed = seed + 2147483647;
      }
      r[i+j*m] = a + ( b - a ) * ( double ) ( seed ) * 4.656612875E-10;
    }
  }

  return r;
}
//****************************************************************************80

double r8vec_diff_norm ( int n, double a[], double b[] )

//****************************************************************************80
//
//  Purpose:
//
//    R8VEC_DIFF_NORM returns the L2 norm of the difference of R8VEC's.
//
//  Discussion:
//
//    An R8VEC is a vector of R8's.
//
//    The vector L2 norm is defined as:
//
//      R8VEC_NORM_L2 = sqrt ( sum ( 1 <= I <= N ) A(I)^2 ).
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    24 June 2011
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of entries in A.
//
//    Input, double A[N], B[N], the vectors.
//
//    Output, double R8VEC_DIFF_NORM, the L2 norm of A - B.
//
{
  int i;
  double value;

  value = 0.0;

  for ( i = 0; i < n; i++ )
  {
    value = value + ( a[i] - b[i] ) * ( a[i] - b[i] );
  }
  value = sqrt ( value );

  return value;
}
//****************************************************************************80

void r8vec_direct_product ( int factor_index, int factor_order,
  double factor_value[], int factor_num, int point_num, double x[] )

//****************************************************************************80
//
//  Purpose:
//
//    R8VEC_DIRECT_PRODUCT creates a direct product of R8VEC's.
//
//  Discussion:
//
//    An R8VEC is a vector of R8's.
//
//    To explain what is going on here, suppose we had to construct
//    a multidimensional quadrature rule as the product of K rules
//    for 1D quadrature.
//
//    The product rule will be represented as a list of points and weights.
//
//    The J-th item in the product rule will be associated with
//      item J1 of 1D rule 1,
//      item J2 of 1D rule 2,
//      ...,
//      item JK of 1D rule K.
//
//    In particular,
//      X(J) = ( X(1,J1), X(2,J2), ..., X(K,JK))
//    and
//      W(J) = W(1,J1) * W(2,J2) * ... * W(K,JK)
//
//    So we can construct the quadrature rule if we can properly
//    distribute the information in the 1D quadrature rules.
//
//    This routine carries out that task.
//
//    Another way to do this would be to compute, one by one, the
//    set of all possible indices (J1,J2,...,JK), and then index
//    the appropriate information.  An advantage of the method shown
//    here is that you can process the K-th set of information and
//    then discard it.
//
//  Example:
//
//    Rule 1:
//      Order = 4
//      X(1:4) = ( 1, 2, 3, 4 )
//
//    Rule 2:
//      Order = 3
//      X(1:3) = ( 10, 20, 30 )
//
//    Rule 3:
//      Order = 2
//      X(1:2) = ( 100, 200 )
//
//    Product Rule:
//      Order = 24
//      X(1:24) =
//        ( 1, 10, 100 )
//        ( 2, 10, 100 )
//        ( 3, 10, 100 )
//        ( 4, 10, 100 )
//        ( 1, 20, 100 )
//        ( 2, 20, 100 )
//        ( 3, 20, 100 )
//        ( 4, 20, 100 )
//        ( 1, 30, 100 )
//        ( 2, 30, 100 )
//        ( 3, 30, 100 )
//        ( 4, 30, 100 )
//        ( 1, 10, 200 )
//        ( 2, 10, 200 )
//        ( 3, 10, 200 )
//        ( 4, 10, 200 )
//        ( 1, 20, 200 )
//        ( 2, 20, 200 )
//        ( 3, 20, 200 )
//        ( 4, 20, 200 )
//        ( 1, 30, 200 )
//        ( 2, 30, 200 )
//        ( 3, 30, 200 )
//        ( 4, 30, 200 )
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    18 April 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int FACTOR_INDEX, the index of the factor being processed.
//    The first factor processed must be factor 0.
//
//    Input, int FACTOR_ORDER, the order of the factor.
//
//    Input, double FACTOR_VALUE[FACTOR_ORDER], the factor values
//    for factor FACTOR_INDEX.
//
//    Input, int FACTOR_NUM, the number of factors.
//
//    Input, int POINT_NUM, the number of elements in the direct product.
//
//    Input/output, double X[FACTOR_NUM*POINT_NUM], the elements of the
//    direct product, which are built up gradually.
//
//  Local Parameters:
//
//    Local, int START, the first location of a block of values to set.
//
//    Local, int CONTIG, the number of consecutive values to set.
//
//    Local, int SKIP, the distance from the current value of START
//    to the next location of a block of values to set.
//
//    Local, int REP, the number of blocks of values to set.
//
{
  static int contig = 0;
  int i;
  int j;
  int k;
  static int rep = 0;
  static int skip = 0;
  int start;

  if ( factor_index == 0 )
  {
    contig = 1;
    skip = 1;
    rep = point_num;
    for ( j = 0; j < point_num; j++ )
    {
      for ( i = 0; i < factor_num; i++ )
      {
        x[i+j*factor_num] = 0.0;
      }
    }
  }

  rep = rep / factor_order;
  skip = skip * factor_order;

  for ( i = 0; i < factor_order; i++ )
  {
    start = 0 + i * contig;

    for ( k = 1; k <= rep; k++ )
    {
      for ( j = start; j < start + contig; j++ )
      {
        x[factor_index+j*factor_num] = factor_value[i];
      }
      start = start + skip;
    }
  }
  contig = contig * factor_order;

  return;
}
//****************************************************************************80

double r8vec_dot_product ( int n, double a1[], double a2[] )

//****************************************************************************80
//
//  Purpose:
//
//    R8VEC_DOT_PRODUCT computes the dot product of a pair of R8VEC's.
//
//  Discussion:
//
//    An R8VEC is a vector of R8's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    03 July 2005
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of entries in the vectors.
//
//    Input, double A1[N], A2[N], the two vectors to be considered.
//
//    Output, double R8VEC_DOT_PRODUCT, the dot product of the vectors.
//
{
  int i;
  double value;

  value = 0.0;
  for ( i = 0; i < n; i++ )
  {
    value = value + a1[i] * a2[i];
  }
  return value;
}
//****************************************************************************80

double *r8vec_linspace_new ( int n, double a_first, double a_last )

//****************************************************************************80
//
//  Purpose:
//
//    R8VEC_LINSPACE_NEW creates a vector of linearly spaced values.
//
//  Discussion:
//
//    An R8VEC is a vector of R8's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    29 March 2011
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of entries in the vector.
//
//    Input, double A_FIRST, A_LAST, the first and last entries.
//
//    Output, double R8VEC_LINSPACE_NEW[N], a vector of linearly spaced data.
//
{
  double *a;
  int i;

  a = new double[n];

  if ( n == 1 )
  {
    a[0] = ( a_first + a_last ) / 2.0;
  }
  else
  {
    for ( i = 0; i < n; i++ )
    {
      a[i] = ( ( double ) ( n - 1 - i ) * a_first 
             + ( double ) (         i ) * a_last ) 
             / ( double ) ( n - 1     );
    }
  }
  return a;
}
//****************************************************************************80

void r8vec_print ( int n, double a[], string title )

//****************************************************************************80
//
//  Purpose:
//
//    R8VEC_PRINT prints an R8VEC.
//
//  Discussion:
//
//    An R8VEC is a vector of R8's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    16 August 2004
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of components of the vector.
//
//    Input, double A[N], the vector to be printed.
//
//    Input, string TITLE, a title.
//
{
  int i;

  cout << "\n";
  cout << title << "\n";
  cout << "\n";
  for ( i = 0; i < n; i++ )
  {
    cout << "  " << setw(8)  << i
         << ": " << setw(14) << a[i]  << "\n";
  }

  return;
}
//****************************************************************************80

double *rbf_interp ( int m, int nd, double xd[], double r0, 
  void phi ( int n, double r[], double r0, double v[] ), double w[], 
  int ni, double xi[] )

//****************************************************************************80
//
//  Purpose:
//
//    RBF_INTERP evaluates a radial basis function interpolant.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    30 June 2012
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    William Press, Brian Flannery, Saul Teukolsky, William Vetterling,
//    Numerical Recipes in FORTRAN: The Art of Scientific Computing,
//    Third Edition,
//    Cambridge University Press, 2007,
//    ISBN13: 978-0-521-88068-8,
//    LC: QA297.N866.
//
//  Parameters:
//
//    Input, int M, the spatial dimension.
//
//    Input, int ND, the number of data points.
//
//    Input, double XD[M*ND], the data points.
//
//    Input, double R0, a scale factor.  R0 should be larger than the typical
//    separation between points, but smaller than the maximum separation.
//    The value of R0 has a significant effect on the resulting interpolant.
//
//    Input, void PHI ( int N, double R[], double R0, double V[] ), a 
//    function to evaluate the radial basis functions.
//
//    Input, double W[ND], the weights, as computed by RBF_WEIGHTS.
//
//    Input, int NI, the number of interpolation points.
//
//    Input, double XI[M*NI], the interpolation points.
//
//    Output, double RBF_INTERP[NI], the interpolated values.
//
{
  double *fi;
  int i;
  int j;
  int k;
  double *r;
  double *v;

  fi = new double[ni];
  r = new double[nd];
  v = new double[nd];

  for ( i = 0; i < ni; i++ )
  {
    for ( j = 0; j < nd; j++ )
    {
      r[j] = 0.0;
      for ( k = 0; k < m; k++ )
      {
        r[j] = r[j] + pow ( xi[k+i*m] - xd[k+j*m], 2 );
      }
      r[j] = sqrt ( r[j] );
    }
    phi ( nd, r, r0, v );

    fi[i] = r8vec_dot_product ( nd, v, w );
  }

  delete [] r;
  delete [] v;

  return fi;
}
//****************************************************************************80

double *rbf_weight ( int m, int nd, double xd[], double r0, 
  void phi ( int n, double r[], double r0, double v[] ), 
  double fd[] )

//****************************************************************************80
//
//  Purpose:
//
//    RBF_WEIGHT computes weights for radial basis function interpolation.
//
//  Discussion:
//
//    We assume that there are N (nonsingular) equations in N unknowns.
//
//    However, it should be clear that, if we are willing to do some kind
//    of least squares calculation, we could allow for singularity,
//    inconsistency, or underdetermine systems.  This could be associated
//    with data points that are very close or repeated, a smaller number
//    of data points than function values, or some other ill-conditioning
//    of the system arising from a peculiarity in the point spacing.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    30 June 2012
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    William Press, Brian Flannery, Saul Teukolsky, William Vetterling,
//    Numerical Recipes in FORTRAN: The Art of Scientific Computing,
//    Third Edition,
//    Cambridge University Press, 2007,
//    ISBN13: 978-0-521-88068-8,
//    LC: QA297.N866.
//
//  Parameters:
//
//    Input, int M, the spatial dimension.
//
//    Input, int ND, the number of data points.
//
//    Input, double XD[M*ND], the data points.
//
//    Input, double R0, a scale factor.  R0 should be larger than the typical
//    separation between points, but smaller than the maximum separation.
//    The value of R0 has a significant effect on the resulting interpolant.
//
//    Input, void PHI ( int N, double R[], double R0, double V[] ), a 
//    function to evaluate the radial basis functions.
//
//    Input, double FD[ND], the function values at the data points.
//
//    Output, double RBF_WEIGHT[ND], the weights.
//
{
  double *a;
  int i;
  int j;
  int k;
  double *r;
  double *v;
  double *w;

  a = new double[nd*nd];
  r = new double[nd];
  v = new double[nd];

  for ( i = 0; i < nd; i++ )
  {
    for ( j = 0; j < nd; j++ )
    {
      r[j] = 0.0;
      for ( k = 0; k < m; k++ )
      {
        r[j] = r[j] + pow ( xd[k+i*m] - xd[k+j*m], 2 );
      }
      r[j] = sqrt ( r[j] );
    }
    phi ( nd, r, r0, v );

    for ( j = 0; j < nd; j++ )
    {
      a[i+j*nd] = v[j];
    }
  }
//
//  Solve for the weights.
//
  w = r8mat_solve_svd ( nd, nd, a, fd );

  delete [] a;
  delete [] r;
  delete [] v;

  return w;
}
//****************************************************************************80

void timestamp ( )

//****************************************************************************80
//
//  Purpose:
//
//    TIMESTAMP prints the current YMDHMS date as a time stamp.
//
//  Example:
//
//    31 May 2001 09:45:54 AM
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    08 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    None
//
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct std::tm *tm_ptr;
  size_t len;
  std::time_t now;

  now = std::time ( NULL );
  tm_ptr = std::localtime ( &now );

  len = std::strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm_ptr );

  std::cout << time_buffer << "\n";

  return;
# undef TIME_SIZE
}
