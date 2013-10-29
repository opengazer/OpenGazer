void daxpy ( int n, double da, double dx[], int incx, double dy[], int incy );
double ddot ( int n, double dx[], int incx, double dy[], int incy );
double dnrm2 ( int n, double x[], int incx );
void drot ( int n, double x[], int incx, double y[], int incy, double c,
  double s );
void drotg ( double *sa, double *sb, double *c, double *s );
void dscal ( int n, double sa, double x[], int incx );
int dsvdc ( double a[], int lda, int m, int n, double s[], double e[], 
  double u[], int ldu, double v[], int ldv, double work[], int job );
void dswap ( int n, double x[], int incx, double y[], int incy );
int i4_max ( int i1, int i2 );
int i4_min ( int i1, int i2 );
int i4_power ( int i, int j );
void phi1 ( int n, double r[], double r0, double v[] );
void phi2 ( int n, double r[], double r0, double v[] );
void phi3 ( int n, double r[], double r0, double v[] );
void phi4 ( int n, double r[], double r0, double v[] );
double r8_abs ( double x );
double r8_max ( double x, double y );
double r8_sign ( double x );
double *r8mat_copy_new ( int m, int n, double a1[] );
double *r8mat_mv_new ( int m, int n, double a[], double x[] );
double *r8mat_solve_svd ( int m, int n, double a[], double b[] );
void r8mat_transpose_print ( int m, int n, double a[], string title );
void r8mat_transpose_print_some ( int m, int n, double a[], int ilo, int jlo,
  int ihi, int jhi, string title );
double *r8mat_uniform_new ( int m, int n, double a, double b, int &seed );
double r8vec_diff_norm ( int n, double a[], double b[] );
void r8vec_direct_product ( int factor_index, int factor_order,
  double factor_value[], int factor_num, int point_num, double x[] );
double r8vec_dot_product ( int n, double a1[], double a2[] );;
double *r8vec_linspace_new ( int n, double a_first, double a_last );
void r8vec_print ( int n, double a[], string title );
double *rbf_interp ( int m, int nd, double xd[], double r0, 
  void phi ( int n, double r[], double r0, double v[] ), double w[], 
  int ni, double xi[] );
double *rbf_weight ( int m, int nd, double xd[], double r0, 
  void phi ( int n, double r[], double r0, double v[] ), 
  double fd[] );
void timestamp ( );
