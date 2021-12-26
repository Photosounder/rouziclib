extern double eval_polynomial(double x, double *c, int degree);
extern xy_t eval_polynomial_xy(xy_t p, xy_t *c, int degree);
extern double eval_polynomial_unrolled(double x, double *c, int degree);
extern xy_t eval_polynomial_unrolled_xy(xy_t p, xy_t *c, int degree);
extern double eval_polynomial_2d(xy_t p, double **c, xyi_t degree);
extern double eval_chebyshev_polynomial(double x, double *cm, int degree);
extern xy_t eval_chebyshev_polynomial_xy(xy_t x, xy_t *cm, int degree);
extern void integrate_chebyshev_coefs(double *cm, int degree, double *cmi, double span);
extern double get_polynomial_error(double (*f)(double), double start, double end, double *c, int degree, int errmode);
extern double get_polynomial_error_2d(double (*f)(double,double), xy_t start, xy_t end, double **c, xyi_t degree, int errmode);
extern double get_polynomial_error_from_points(double *x, double *y, int p_count, double *c, int degree, int errmode);
extern char *print_polynomial(double *c, int degree, const char *x);

extern void polynomial_addition(double *a, int adeg, double *b, int bdeg, double *c);
extern void polynomial_subtraction(double *a, int adeg, double *b, int bdeg, double *c);
extern void polynomial_scalar_mul(const double *a, int adeg, double m, double *c);
extern void polynomial_scalar_mul_2d(const double **a, xyi_t adeg, double m, double **c);
extern void polynomial_scalar_muladd_2d(const double **a, xyi_t adeg, double m, double **c);
extern int polynomial_multiplication(double *a, int adeg, double *b, int bdeg, double *c, int cdeg);
extern xyi_t polynomial_multiplication_2d(double **a, xyi_t adeg, double **b, xyi_t bdeg, double **c, xyi_t cdeg);
extern double *polynomial_power(double *a, int adeg, int n, int *maxdegp);
extern double **polynomial_power_2d(double **a, xyi_t adeg, int n, xyi_t *maxdegp);
extern void polynomial_x_substitution(double *a, int adeg, double *xs, int xsdeg, double *c);

extern double *chebyshev_coefs(int degree);
extern const double *chebyshev_coefs_cached(int degree);
extern double **chebyshev_coefs_2d(xyi_t degree);
extern double chebyshev_node(double degree, double node);

extern void polynomial_fit_on_points(xy_t *p, double *c, int degree);
extern void polynomial_fit_on_function(double (*f)(double), double start, double end, double *c, int degree);
extern double chebyshev_multiplier_by_dct(double *y, int p_count, int id, double (*cos_func)(double));
extern double chebyshev_multiplier_by_dct_2d(double **z, int p_count, xyi_t id);
extern void polynomial_fit_on_points_by_dct(double *y, int p_count, double start, double end, double *c, int degree, double (*cos_func)(double));
extern double *polynomial_function_to_points(double (*f)(double), double start, double end, int p_count);
extern void polynomial_fit_on_function_by_dct_count(double (*f)(double), double start, double end, double *c, int degree, int p_count, double (*cos_func)(double));
extern void polynomial_fit_on_function_by_dct(double (*f)(double), double start, double end, double *c, int degree, double (*cos_func)(double));
extern void chebyshev_analysis_on_function(double (*f)(double), double start, double end, double *cm, int degree, int p_count, double (*cos_func)(double));
extern void chebyshev_coefs_to_polynomial_2d(double **cm, xyi_t degree, xy_t start, xy_t end, double **c);
extern double **chebyshev_fit_on_points_by_dct_2d(double **z, int p_count, xyi_t degree);
extern double **polynomial_fit_on_points_by_dct_2d(double **z, int p_count, xy_t start, xy_t end, double **c, xyi_t degree);
extern double **polynomial_function_to_point_2d(double (*f)(double, double), int p_count, xy_t start, xy_t end);
extern double **polynomial_fit_on_function_by_dct_2d(double (*f)(double, double), xy_t start, xy_t end, double **c, xyi_t degree);
extern void polynomial_fit_on_function_by_dct_minmax(double (*f)(double), double start, double end, double *c, int degree);
extern double reduce_digits(double (*f)(double), double segstart, double segend, double *c, const int degree, int errmode, double added_error_thresh, double digits);
extern double reduce_digits_2d(double (*f)(double,double), xy_t segstart, xy_t segend, double **c, const xyi_t degree, int errmode, double added_error_thresh, double digits);

enum { NEGMODE, DIVMODE };
