extern double erf_radlim_mid_value(double k);
extern double erf_radlim_lim0_approx(double x);
extern double erf_radlim_lim0_log_approx(double x);
extern double erf_radlim_liminf_approx(double x);
extern double erf_radlim_liminf_log_approx(double x);
extern double erf_radlim_lim0_weight(double k);
extern double erf_radlim_liminf_weight(double k);
extern double erf_radlim_approx(double x, double k);
extern double erf_radlim_approx_gpu(double x, double k);

extern double calc_triangle_pixel_weight(triangle_t tr, xy_t p, double drawing_thickness);
