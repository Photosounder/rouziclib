extern double eval_polynomial(double x, double *c, int degree);
extern void eval_polynomial_mpfr(real_t y, real_t x, real_t *c, int degree);
extern double get_polynomial_error(double (*f)(double), double start, double end, double *c, int degree, int errmode);
extern double get_polynomial_error_from_points(double *x, double *y, int p_count, double *c, int degree, int errmode);
extern char *print_polynomial(double *c, int degree, const char *x);

extern void polynomial_addition(double *a, int adeg, double *b, int bdeg, double *c);
extern void polynomial_subtraction(double *a, int adeg, double *b, int bdeg, double *c);
extern void polynomial_scalar_mul(double *a, int adeg, double m, double *c);
extern int polynomial_multiplication(double *a, int adeg, double *b, int bdeg, double *c, int cdeg);
extern double *polynomial_power(double *a, int adeg, int n, int *maxdegp);
extern void polynomial_x_substitution(double *a, int adeg, double *xs, int xsdeg, double *c);
extern void polynomial_addition_mpfr(real_t *a, int adeg, real_t *b, int bdeg, real_t *c);
extern void polynomial_subtraction_mpfr(real_t *a, int adeg, real_t *b, int bdeg, real_t *c);
extern void polynomial_scalar_mul_mpfr(real_t *a, int adeg, real_t m, real_t *c);
extern int polynomial_multiplication_mpfr(real_t *a, int adeg, real_t *b, int bdeg, real_t *c, int cdeg);
extern real_t *polynomial_power_mpfr(real_t *a, int adeg, int n, int *maxdegp);
extern void polynomial_x_substitution_mpfr(real_t *a, int adeg, real_t *xs, int xsdeg, real_t *c);

extern double *chebyshev_coefs(int degree);
extern real_t *chebyshev_coefs_mpfr(int degree);
extern double chebyshev_node(double degree, double node);
extern void chebyshev_node_mpfr(real_t v, double degree, double node);

extern void polynomial_fit_on_points(xy_t *p, double *c, int degree);
extern void polynomial_fit_on_points_mpfr(real_t *x, real_t *y, real_t *c, int degree);
extern void polynomial_fit_on_function(double (*f)(double), double start, double end, double *c, int degree);
extern void polynomial_fit_on_function_mpfr(void (*f)(real_t,real_t), real_t start, real_t end, real_t *c, int degree);
extern double chebyshev_multiplier_by_dct(double *y, int p_count, int id);
extern void polynomial_fit_on_points_by_dct(double *y, int p_count, double start, double end, double *c, int degree);
extern void polynomial_fit_on_function_by_dct(double (*f)(double), double start, double end, double *c, int degree);
extern void polynomial_fit_on_function_by_dct_minmax(double (*f)(double), double start, double end, double *c, int degree);
extern void chebyshev_multiplier_by_dct_mpfr(real_t v, real_t *y, int p_count, int id);
extern void polynomial_fit_on_points_by_dct_mpfr(real_t *y, int p_count, real_t start, real_t end, real_t *c, int degree);
extern void polynomial_fit_on_function_by_dct_mpfr(void (*f)(real_t,real_t), real_t start, real_t end, real_t *c, int degree);
extern double reduce_digits(const int order, double (*f)(double), double segstart, double segend, double *c, int errmode, double added_error_thresh, double digits);
extern double reduce_digits_mpfr(const int order, void (*f)(real_t,real_t), real_t segstart, real_t segend, real_t *c, int errmode, double added_error_thresh, double digits);

enum { NEGMODE, DIVMODE };
