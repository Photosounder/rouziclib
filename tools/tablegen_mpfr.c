//	gcc tablegen_mpfr.c -o tablegen_mpfr.exe -std=c99 -lm -lmpfr -lgmp -lwinmm -lcomdlg32 -lole32 -Wno-incompatible-pointer-types -O0 -DRL_STOREU_SI32 && ./tablegen_mpfr.exe

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
//#include <windows.h>

#define RL_MPFR
#include "../rouziclib/rouziclib.h"
#include "../rouziclib/rouziclib.c"

#define PREC	512

#define COEF_COUNT 100

double get_double_impl_error(void (*f)(real_t,real_t), double (*g)(double), real_t start, real_t end, int errmode)
{
	int32_t i;
	static int init=1;
	static real_t a, b;
	static real_t x, err;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(b);
		r_init(x);
		r_init(err);
	}

	r_setd(err, 0.);
	for (i=0; i<=1000; i++)
	{
		r_mix(x, (double) i / 1000., start, end);

		r_setd(a, g(r_todouble(x)));	// double implementation
		f(b, x);

		if (errmode==DIVMODE)
		{
			if (mpfr_cmp(a, b) < 0)		//if (a < b)
			{
				r_rdiv(a, b, a);	//a = b / a - 1.;
				r_subd(a, 1.);
			}
			else
			{
				r_div(a, b);		//a = a / b - 1.;
				r_subd(a, 1.);
			}
		}
		else
			r_rsub(a, b, a);		//a = b - a;

		r_abs(a);

		if (mpfr_cmp(err, a) < 0)
			r_set(err, a);
	}

	return r_todouble(err);
}

double get_float_impl_error(void (*f)(real_t,real_t), float (*g)(float), real_t start, real_t end, int errmode)
{
	int32_t i;
	static int init=1;
	static real_t a, b;
	static real_t x, err;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(b);
		r_init(x);
		r_init(err);
	}

	r_setd(err, 0.);
	for (i=0; i<=1000; i++)
	{
		r_mix(x, (double) i / 1000., start, end);

		r_setd(a, g(r_todouble(x)));	// float implementation
		f(b, x);

		if (errmode==DIVMODE)
		{
			if (mpfr_cmp(a, b) < 0)		//if (a < b)
			{
				r_rdiv(a, b, a);	//a = b / a - 1.;
				r_subd(a, 1.);
			}
			else
			{
				r_div(a, b);		//a = a / b - 1.;
				r_subd(a, 1.);
			}
		}
		else
			r_rsub(a, b, a);		//a = b - a;

		r_abs(a);

		if (mpfr_cmp(err, a) < 0)
			r_set(err, a);
	}

	return r_todouble(err);
}

double f_cosd(double x)
{
	return cos(2.*pi*x);
}

float f_cosf(float x)
{
	return cosf(2.*pi*x);
}

void f_cos(real_t y, real_t x)
{
	static int init=1;
	static real_t a;

	if (init)
	{
		init = 0;
		r_init(a);
	}

	r_pi(a);
	r_muld(a, 2.);
	r_mul(a, x);

	r_cos(y, a);
}

double find_max_intermediary_value(real_t *c_r, real_t segstart_r, real_t segend_r)
{
	int32_t i, ic;
	double c[COEF_COUNT], start=r_todouble(segstart_r), end=r_todouble(segend_r), x, v, maxv=0.;

	for (ic=0; ic<COEF_COUNT; ic++)
		c[ic] = r_todouble(c_r[ic]);

	for (i=0; i<=1000; i++)		// evaluate the polynomial for many different values of x in the given range
	{
		x = (double) i / 1000. * (end-start) + start;

		if (fabs(x) > maxv) maxv = fabs(x);
		v = c[COEF_COUNT-1];
		if (fabs(v) > maxv) maxv = fabs(v);
		for (ic=COEF_COUNT-2; ic>=0; ic--)
		{
			v *= x;
			if (fabs(v) > maxv) maxv = fabs(v);
			v += c[ic];
			if (fabs(v) > maxv) maxv = fabs(v);
			if (fabs(c[ic]) > maxv) maxv = fabs(c[ic]);
		}
	}

	return maxv;
}

void make_cos_table(const int cos_order, const int lutsp, const int quads)	// cos_order is <= 6, lutsp can only be <= 15
{
	int32_t i, is, segcount, prec;
	real_t c[COEF_COUNT];
	real_t start, segstep, segstart, segend;
	double end, step, err, errd, errf, maxerr=0., maxerrd=0., maxerrf=0., v, maxv=0.;
double k=0., errmin=1., mink;
	FILE *file;
	char header_path[64];

	sprintf(header_path, "../rouziclib/approximations/tables/fastcos_d%d.h", cos_order);
	file = fopen(header_path, "wb");
	fprintf(file, "{");

	end = (double) quads * 0.25;
	step = 0.25 / pow(2., (double) lutsp);
	segcount = ceil(end / step);
	if (quads < 4)
		segcount++;	// +1 for the only lookup for x==0.25 or x==0.5

	r_init(start);
	r_init(segstep);
	r_setd(segstep, step);
	r_init(segstart);
	r_init(segend);

	for (i=0; i<COEF_COUNT; i++)
		r_init(c[i]);

	for (is=0; is<segcount; is++)
	{
		r_set(segstart, segend);
		r_add(segend, segstep);

		//if (is==segcount-1)	// only useful if the last segment only needs to cover a shorter range than usual
		//	r_setd(segend, end);

		for (i=0; i<COEF_COUNT; i++)
			r_setd(c[i], 0.);

		if (quads==2 && is==segcount-1)		// if the final segment is at x = 0.5
			r_setd(c[0], -1);

		if (quads==4 || is!=segcount-1)		// let the final segment be 0
		{
			polynomial_fit_on_function_by_dct_mpfr(f_cos, segstart, segend, c, cos_order);
			err = get_polynomial_error_mpfr(f_cos, segstart, segend, c, cos_order, NEGMODE);
			if (err > maxerr)
				maxerr = err;
		}

		errd = get_double_impl_error(f_cos, f_cosd, segstart, segend, NEGMODE);
		if (errd > maxerrd)
			maxerrd = errd;

		errf = get_float_impl_error(f_cos, f_cosf, segstart, segend, NEGMODE);
		if (errf > maxerrf)
			maxerrf = errf;

		v = find_max_intermediary_value(c, segstart, segend);
		if (v > maxv)
			maxv = v;

		//if (is==segcount/2)
		/*for (i=cos_order; i>=1; i--)
			if (i==1)
				mpfr_printf("%+.16Rf*x", c[i]);
			else
				mpfr_printf("%+.16Rf*x^%d", c[i], i);
		mpfr_printf("%+.16Rf\n[%.7Rf , %.7Rf]\terr %g\n", c[0], segstart, segend, err);*/
		//mpfr_printf("%+.16Rf  err %g\n\tdouble err:%g\ti=%3d (segratio 1/%.0f) [%.7Rf , %.7Rf]\n", c[0], err, errd, is, 1./step, segstart, segend);

		for (i=0; i<cos_order; i++)
			mpfr_fprintf(file, "%.17Rg, ", c[i]);
		mpfr_fprintf(file, "%.17Rg%s", c[cos_order], (is==segcount-1 && quads < 4) ? "};\n" : ", \n");
	}

	if (quads==4)
	{
		for (i=0; i<cos_order; i++)
			mpfr_fprintf(file, (i==0) ? "1., " : "0., ");
		mpfr_fprintf(file, "0.};\n");
	}

	fprintf(file, "const uint32_t lutsp = %d;\n", lutsp);
	fprintf(file, "const int quads = %d;", quads);

	fclose (file);

	if (maxv <= 2047.)
		prec = 63 - ffo_lut[(int) ceil(maxv)];
	else
		prec = -1;
	printf("\nMax error: %g\tMax double implementation error: %g, float implementation error: %g\nMax intermediary value: %g, prec: %d fractional bits\n", maxerr, maxerrd, maxerrf, maxv, prec);
}

void make_cos_table_human(const int cos_order, const double step)	// cos_order is <= 6
{
	int32_t i, is, segcount;
	real_t c[COEF_COUNT];
	real_t segstep, segstart, segend, maxseg;
	double end=1., err, maxerr=0.;
	FILE *file;
	char header_path[64];

	segcount = nearbyint(ceil(end / step));

	sprintf(header_path, "cos_d%d_step_%g_human new.dat", cos_order, step);
	file = fopen(header_path, "wb");

	r_init(segstep);
	r_setd(segstep, step);
	r_init(segstart);
	r_init(segend);
	r_init(maxseg);

	for (i=0; i<COEF_COUNT; i++)
		r_init(c[i]);

	for (is=0; is<segcount; is++)
	{
		r_set(segstart, segend);
		r_add(segend, segstep);

		for (i=0; i<COEF_COUNT; i++)
			r_setd(c[i], 0.);

		// Fit
		polynomial_fit_on_function_by_dct_mpfr(f_cos, segstart, segend, c, cos_order);
		//err = get_polynomial_error_mpfr(f_cos, segstart, segend, c, cos_order, NEGMODE);

		// Reduce digits
		err = reduce_digits_mpfr(f_cos, segstart, segend, c, cos_order, NEGMODE, 1.1, 30.);
		if (err > maxerr)
		{
			maxerr = err;
			r_set(maxseg, segstart);
		}

		// Print to file
		mpfr_fprintf(file, "%Rg	", segstart);
		for (i=cos_order; i > 0; i--)
			mpfr_fprintf(file, "% .17Rg   ", c[i]);
		mpfr_fprintf(file, "% .17Rg\n", c[0]);
		fflush(file);
	}
	mpfr_fprintf(file, "\nMax error: %g at segment %Rg\n", maxerr, maxseg);

	fclose (file);

	printf("\nMax error: %g\n", maxerr);
}

void compare_chebyshev_arithmetics(void (*f)(real_t,real_t), real_t *cm, int degree, double xd)
{
	int id;
	real_t xr, yr, x2r;
	ddouble_t xq={0};

	xd *= 4.;

	r_init(xr);
	r_init(yr);
	r_setd(xr, xd);
	xq.hi = xd;

	ddouble_t b1q={0}, b2q, yq, x2q = mul_qd_simple(xq, 2.);
	real_t b1r, b2r;

	r_init(b1r);
	r_init(b2r);
	r_init(x2r);
	r_rmuld(x2r, xr, 2.);

	// Clenshaw evaluation
	yq = mpfr_to_ddouble(cm[degree]);
	r_set(yr, cm[degree]);
	for (id = degree-1; id >= 1; id--)
	{
		fprintf(stdout, "id %2d:\n", id);

		b2q = b1q;
		b1q = yq;
		r_set(b2r, b1r);
		r_set(b1r, yr);

		//y = cm[id] + x2*b1 - b2q;

		yq = mul_qq(x2q, b1q);
		r_rmul(yr, x2r, b1r);
		mpfr_fprintf(stdout, "	x2 %Rg (diff %g) * b1 %Rg (diff %g) => y %Rg (diff %g)\n", x2r, sub_qq(x2q, mpfr_to_ddouble(x2r)).hi, b1r, sub_qq(b1q, mpfr_to_ddouble(b1r)).hi, yr, sub_qq(yq, mpfr_to_ddouble(yr)).hi);

		yq = add_qq(mpfr_to_ddouble(cm[id]), yq);
		r_add(yr, cm[id]);
		mpfr_fprintf(stdout, "	cm[%d] %Rg + y => y %Rg (diff %g)\n", id, cm[id], yr, sub_qq(yq, mpfr_to_ddouble(yr)).hi);

		yq = sub_qq(yq, b2q);
		r_sub(yr, b2r);
		mpfr_fprintf(stdout, "	y - b2 %Rg (diff %g) => y %Rg (diff %g)\n", b2r, sub_qq(b2q, mpfr_to_ddouble(b2r)).hi, yr, sub_qq(yq, mpfr_to_ddouble(yr)).hi);
	}

	//y = cm[0] + x*y - b1q;
	yq = mul_qq(xq, yq);
	yq = add_qq(mpfr_to_ddouble(cm[0]), yq);
	yq = sub_qq(yq, b1q);

	r_mul(yr, xr);
	r_add(yr, cm[0]);
	r_sub(yr, b1r);

//yq = cos_tr_q(mul_qd_simple(xq, 0.25));
	// Compare yr with yq
	mpfr_fprintf(stdout, "id %2d: %g\n", id, sub_qq(yq, mpfr_to_ddouble(yr)).hi);

	r_free(b1r);
	r_free(b2r);
	r_free(xr);
	r_free(yr);
	r_free(x2r);
}

void make_cos_double_double_approx()
{
	int i;
	const int degree = 28, p_count = 1000;
	real_t *c, *cm;
	real_t start, end, v;
	double err;

	cm = r_init_array(degree+1);
	c = r_init_array(degree+1);

	r_init(start);
	r_setd(start, -0.25);
	r_init(end);
	r_setd(end, 0.25);
	r_init(v);

	chebyshev_analysis_on_function_mpfr(f_cos, start, end, cm, degree, p_count);
	polynomial_fit_on_function_by_dct_mpfr(f_cos, start, end, c, degree);
	err = get_polynomial_error_mpfr(f_cos, start, end, c, degree, NEGMODE);
	fprintf(stdout, "Polynomial error: %.5eg\n", err);

	// Print Chebyshev multipliers
	for (i=0; i <= degree; i+=2)
	{
		mpfr_abs(v, cm[i], MPFR_RNDN);
		if (mpfr_cmp_d(v, 1e-150) > 0)
		{
			//mpfr_fprintf(stdout, "T_%d(x) * % .32Rg\n", i, cm[i]);
			ddouble_t q = mpfr_to_ddouble(cm[i]);
			fprintf(stdout, "{%.17g, %.17g},	// T_%d (err %.3g)\n", q.hi, q.lo, i, diff_mpfr_ddouble(cm[i], q));
		}
	}

	// Print polynomial coefficients
	for (i=0; i <= degree; i+=2)
	{
		ddouble_t q = mpfr_to_ddouble(c[i]);
		fprintf(stdout, "{%.17g, %.17g},	// c%d\n", q.hi, q.lo, i);
	}

	// Print Horner form of polynomial
	for (i=degree; i > 2; i-=2)
		mpfr_fprintf(stdout, "(");
	for (i=degree; i > 0; i-=2)
		if (i==degree)
			mpfr_fprintf(stdout, "%.17Rg*x2 + ", c[i]);
		else
			mpfr_fprintf(stdout, "%.17Rg)*x2 + %s", c[i], (i%8) ? "" : "\n\t");
	mpfr_fprintf(stdout, "%.17Rg\n", c[0]);

	// Compare arithmetics
	compare_chebyshev_arithmetics(f_cos, cm, degree, 0.24998772);

	r_free(v);
	r_free_array(&cm, degree+1);
	r_free_array(&c, degree+1);
}

void test_cos_ddouble()
{
	int i;
	real_t x, ym, yq;
	ddouble_t r;
	double err, maxerr=0.;

	r_init(x);
	r_init(ym);
	r_init(yq);

	r_setd(x, 0.);
	for (; mpfr_cmp_d(x, 0.25) < 0; r_addd(x, 1.18e-7))
	{
		ddouble_to_mpfr(x, mpfr_to_ddouble(x));		// round it to an exact double-double value

		f_cos(ym, x);
		r = cos_tr_q(mpfr_to_ddouble(x));
		ddouble_to_mpfr(yq, r);
		r_sub(yq, ym);
		r_abs(yq);
		err = r_todouble(yq);
		maxerr = MAXN(err, maxerr);
		if (err > 3e-32)
			fprintf(stdout, "f(%.17g): %.5e\n", r_todouble(x), err);
	}

	fprintf(stdout, "Max error of cos_tr_q(): %.5e\n", maxerr);

	r_free(x);
	r_free(ym);
	r_free(yq);
}

// Sinc windowed with squared gaussian window

void r_squared_gaussian_window(real_t y, real_t x, real_t w)
{
	static int init=1;
	static real_t a, b;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(b);
	}

	//    	sq(gaussian(x*w) - gaussian(w)) / sq(gaussian(0.) - gaussian(w));
	//   =	sq(gaussian(x*w) - gaussian(w)) / sq(1. - gaussian(w));
	//   a = x*w
	r_rmul(a, x, w);

	//   =	sq(gaussian(a) - gaussian(w)) / sq(1. - gaussian(w));
	//   a = gaussian(a)
	//   b = gaussian(w)
	r_gaussian(a, a);
	r_gaussian(b, w);

	//   =  sq(a - b) / sq(1. - b)
	//   a = a - b
	r_sub(a, b);
	//   b = 1 - b
	r_setd(y, 1.);
	r_rsub(b, y, b);

	//   a = sq(a)
	//   b = sq(b)
	r_sq(a, a);
	r_sq(b, b);

	//   y = a / b
	r_rdiv(y, a, b);
}

real_t ws_range, ws_sigma_range;
int no_sine=0;

void f_wsinc(real_t y, real_t x)
{
	static int init=1;
	static real_t a;

	if (init)
	{
		init = 0;
		r_init(a);
	}

	r_abs(x);
	if (mpfr_cmp(x, ws_range) >= 0)	// if x is outside of the wsinc range
	{
		r_setd(y, 0.);
		return ;
	}

	if (mpfr_cmp_d(x, 0.)==0)	// if x == 0
	{
		r_setd(y, 1.);
		return ;
	}

	if (no_sine)
		r_setd(y, 1.);
	else
	{
		// sin(pi*x)
		r_pi(a);
		r_mul(a, x);
		r_sin(y, a);
	}

	// * 1/(pi*x)
	r_pi(a);
	r_mul(a, x);
	r_div(y, a);

	// Windowing
	r_rdiv(a, x, ws_range);
	r_squared_gaussian_window(a, a, ws_sigma_range);
	r_mul(y, a);
}

void make_wsinc_table(const int order, const int lutsp, double wsinc_range, double sigma_range)	// order is <= 6, lutsp can only be <= 15
{
	int32_t i, is, segcount, prec;
	real_t c[COEF_COUNT];
	real_t segstep, segstart, segend;
	double end=wsinc_range, step=1./pow(2., lutsp), err, maxerr=0.;
	FILE *file;
	char header_path[64];

	sprintf(header_path, "../rouziclib/approximations/tables/fastwsinc.h", order);
	file = fopen(header_path, "wb");
	fprintf(file, "{");
	
	r_init(ws_range);
	r_setd(ws_range, wsinc_range);
	r_init(ws_sigma_range);
	r_setd(ws_sigma_range, sigma_range);

	segcount = ceil(end / step);

	r_init(segstep);
	r_setd(segstep, step);
	r_init(segstart);
	r_init(segend);

	for (i=0; i<COEF_COUNT; i++)
		r_init(c[i]);

	for (is=0; is<segcount; is++)
	{
		r_set(segstart, segend);
		r_add(segend, segstep);

		if (is==segcount-1)	// only useful if the last segment only needs to cover a shorter range than usual
			r_setd(segend, end);

		for (i=0; i<COEF_COUNT; i++)
			r_setd(c[i], 0.);

		//if (r_todouble(segstart)==1.)
		//	no_sine = 1;

		// Fitting
		polynomial_fit_on_function_by_dct_mpfr(f_wsinc, segstart, segend, c, order);
		err = get_polynomial_error_mpfr(f_wsinc, segstart, segend, c, order, no_sine==0 ? NEGMODE : DIVMODE);
		if (err > maxerr)
			maxerr = err;

		// Printing
		mpfr_printf("[%.7Rf , %.7Rf]\terr %e\n", segstart, segend, err);

		for (i=0; i<order; i++)
			mpfr_fprintf(file, "%.17Rg, ", c[i]);
		mpfr_fprintf(file, "%.17Rg%s", c[order], (is==segcount-1) ? "};\n" : ", \n");
	}

	double ind_off = exp2(double_get_exponent(wsinc_range));	// offset to calculate the LUT index from the offset mantissa
	if (double_get_mantissa(wsinc_range))
		ind_off *= 2.;

	int ish = 52 - (double_get_exponent(ind_off) + lutsp);

	fprintf(file, "const uint32_t order = %d, lutsp = %d, ish = %d;\n", order, lutsp, ish);
	fprintf(file, "const double wsinc_range = %g, ind_off = %g;", wsinc_range, ind_off);

	fclose (file);

	printf("\nMax error: %g\n", maxerr);
	printf("LUT size (double): %zu bytes\n", segcount * (order+1) * sizeof(double));
}

void test_matrix()
{
	double /*a[COEF_COUNT*COEF_COUNT],*/ b[COEF_COUNT*COEF_COUNT], c[COEF_COUNT*COEF_COUNT];
	double a[] = {3., 3.2, 3.5, 3.6};

	matrix_inverse(a, b, 2);
	a[0] = 118.4;
	a[1] = 135.2;
	matrix_multiplication(b, a, c, 2, 2, 1);

	printf("\n\nInverse matrix:\n%4f\t%4f\n%4f\t%4f\n\n", b[0], b[1], b[2], b[3]);
	printf("\n\nResult matrix:\n%4f\t%4f\n", c[0], c[1]);
}

void cos_error_curve()
{
	double x;
	real_t xr, vd, vr, d;

	r_init(xr);
	r_init(vd);
	r_init(vr);
	r_init(d);

	FILE *file = fopen("error_curves/cosd_err.dat", "wb");

	for (x=0.; x <= 2*pi; x += 1e-4)
	{
		r_setd(xr, x);
		r_setd(vd, cos(x));
		r_cos(vr, xr);
		r_rsub(d, vd, vr);
		mpfr_fprintf(file, "%g\t% Rg\n", x, d);
		//mpfr_fprintf(file, "%g\t% g\n", x, r_todouble(vd) - r_todouble(vr));
	}

	fclose(file);
}

// erf_radlim

double radlim, seg_offset=0.;

double erf_radlim(double x, double in_radlim, double in_step)
{
	int i;
	static double step=0.;
	static double radlim=0.;
	static double *v=NULL;
	static size_t v_count=0, v_as=0;
	double t, v_prev;

	if (in_radlim <= 0. || in_step <= 0.)
		return NAN;

	if (radlim != in_radlim || step != in_step)
	{
		radlim = in_radlim;
		step = in_step;

		v_count = 0;
		v_prev = 0.;
		for (i=0, t=-radlim; t < radlim; t+=step, i++)
		{
			alloc_enough(&v, v_count = i+1, &v_as, sizeof(double), 2.);
			if (i > 0)
				v_prev = v[i-1];
			v[i] = v_prev + erf(radlim*sqrt(1.-sq(t/radlim))) * gaussian(t) * step / sqrt(pi);
		}
	}

	double p = (x + radlim) / step;
	int i0, i1;
	i0 = rangelimit_i32(p, 0, v_count-1);
	i1 = rangelimit_i32(p+1., 0, v_count-1);
	t = p - floor(p);
	return v[i0] * (1.-t) + v[i1] * t;
}

double radlim_x_remap(double x)		// remaps x from [-1 , 1] to [-radlim , radlim] non-linearly
{
	double xa, xn;

	x += seg_offset;		// remaps [0 , step] to within [-1 , 1]

	// Delinearise
	xa = fabs(x);
	xn = x;
	if (radlim <= 1.66)
		//xn = (3. - x*x) * x * 0.5;
		xn = (2.*xa - sq(xa)) * sign(x);	// inverse of this is (1. - sqrt(1. - fabs(x))) * sign(x)

	xn *= radlim;			// remaps [-1 , 1] to [-radlim , radlim]

	return xn;
}

void f_erf_radlim(real_t y, real_t x)
{
	r_setd(y, erf_radlim(radlim_x_remap(r_todouble(x)), radlim, 1e-7 * radlim));
}

/*int order_index_glo, order_glo;
real_t zero_glo, segstep;

void f_erf_radlim_coef(real_t y, real_t x)
{
	real_t c[COEF_COUNT];
	for (i=0; i<COEF_COUNT; i++)
		r_init(c[i]);

	find_polynomial_fit_old(f_erf_radlim, zero_glo, segstep_glo, c, order_glo, 0.);

	r_set(y, c[order_index_glo]);

	for (i=0; i<COEF_COUNT; i++)
		r_free(c[i]);
}

void make_erf_radlim_table2(const int order, const double step, int order_index)
{
	int32_t i, is, segcount, prec;
	real_t c[COEF_COUNT];
	real_t segstart, segend, maxseg;
	double start=0., end=1.66, err, maxerr=0.;
	FILE *file;
	char header_path[64];

	segcount = nearbyint(ceil((end-start) / step));

	sprintf(header_path, "erf_radlim2_%g_d%d_step_%g_human.dat", radlim, order, step);
	file = fopen(header_path, "wb");

	r_init(segstep);
	r_setd(segstep, step);
	r_init(segstart);
	r_init(segend);
	r_init(maxseg);
	r_init(zero_glo);

	order_index_glo = order_index;
	order_glo = 3;

	r_setd(segend, start);

	for (i=0; i<COEF_COUNT; i++)
		r_init(c[i]);

	for (is=0; is<segcount; is++)
	{
		r_set(segstart, segend);
		r_add(segend, segstep);

		for (i=0; i<COEF_COUNT; i++)
			r_setd(c[i], 0.);

		seg_offset = r_todouble(segstart);

		// Fit
		find_polynomial_fit_old(f_erf_radlim_coef, segstart, segend, c, order, 0.);
		err = get_polynomial_error_mpfr(f_erf_radlim_coef, segstart, segend, c, order, NEGMODE);

		// Reduce digits
		err = reduce_digits_mpfr(f_erf_radlim_coef, segstart, segend, c, order, NEGMODE, 1.01, 10.);
		if (err > maxerr)
		{
			maxerr = err;
			r_set(maxseg, segstart);
		}

		// Print to file
		mpfr_fprintf(file, "%Rg	", segstart);
		for (i=order; i > 0; i--)
			mpfr_fprintf(file, "% .17Rg   ", c[i]);
		mpfr_fprintf(file, "% .17Rg (err % .3g)\n", c[0], err);
	}
	mpfr_fprintf(file, "\nMax error: %g at segment %Rg\n", maxerr, maxseg);

	fclose (file);

	printf("\nMax error: %g\n", maxerr);
}*/

void make_erf_radlim_table(const int order, const double step)	// order is <= 6
{
	int32_t i, is, segcount, prec;
	real_t c[COEF_COUNT];
	real_t segstep, segstart, segend, maxseg, z;
	double start=-1., end=1., err, maxerr=0.;
	FILE *file;
	char header_path[64];

	segcount = nearbyint(ceil((end-start) / step));

	sprintf(header_path, "erf_radlim_%g_d%d_step_%g_human.dat", radlim, order, step);
	file = fopen(header_path, "wb");

	r_init(segstep);
	r_setd(segstep, step);
	r_init(segstart);
	r_init(segend);
	r_init(maxseg);
	r_init(z);

	r_setd(segend, start);

	for (i=0; i<COEF_COUNT; i++)
		r_init(c[i]);

	for (is=0; is<segcount; is++)
	{
		r_set(segstart, segend);
		r_add(segend, segstep);

		for (i=0; i<COEF_COUNT; i++)
			r_setd(c[i], 0.);

		seg_offset = r_todouble(segstart);

		// Fit
		polynomial_fit_on_function_by_dct_mpfr(f_erf_radlim, z, segend, c, order);
		err = get_polynomial_error_mpfr(f_erf_radlim, z, segstep, c, order, NEGMODE);

		// Reduce digits
		//err = reduce_digits_mpfr(f_erf_radlim, segstart, segend, c, order, NEGMODE, 1.01, 10.);
		reduce_digits_mpfr(f_erf_radlim, z, segstep, c, order, NEGMODE, 1.01, 10.);
		if (err > maxerr)
		{
			maxerr = err;
			r_set(maxseg, segstart);
		}

		// Print to file
		mpfr_fprintf(file, "%Rg	", segstart);
		for (i=order; i > 0; i--)
			mpfr_fprintf(file, "% .17Rg   ", c[i]);
		mpfr_fprintf(file, "% .17Rg (err % .3g)\n", c[0], err);
	}
	mpfr_fprintf(file, "\nMax error: %g at segment %Rg\n", maxerr, maxseg);

	fclose (file);

	printf("\nMax error: %g\n", maxerr);
}

xy_t *xy_array=NULL;
size_t xy_array_size=0;

void f_erf_radlim_mid(real_t y, real_t k)
{
	/*if ((rand() & 0xFF) == 0)
	{
		fprintf_rl(stdout, "radlim = %g\n", r_todouble(k));
		fflush(stdout);
	}

	if (r_todouble(k) < 1e-6)
		r_setd(y, 0.);
	else
		r_setd(y, erf_radlim(r_todouble(k), r_todouble(k), 2e-6 * r_todouble(k)));*/
	r_setd(y, get_interpolated_xy_array_value(r_todouble(k), xy_array, xy_array_size));
}

void load_erf_radlim_mid_raw_lut()
{
	FILE *file;
	char path[] = "erf_radlim_mid_raw.dat";
	int i;
	double k;
	double ts=0.;
	get_time_diff_hr(&ts);

	if (check_file_is_readable(path))
		parse_xy_array_file(path, &xy_array, &xy_array_size);
	else
	{
		file = fopen_utf8(path, "wb");
		xy_array = calloc(xy_array_size=ceil(4./0.001)+1., sizeof(xy_t));

		for (i=0; i < xy_array_size; i++)
		{
			k = 4. * (double) i / (double) (xy_array_size-1);
			if (k==0.)
				xy_array[i] = xy(k, 0.);
			else
				xy_array[i] = xy(k, 0.5*erf_radlim(k, k, 1e-8 * k));

			if (file)
				fprintf(file, "%.3f\t%.10f\n", xy_array[i].x, xy_array[i].y);
		}

		if (file)
			fclose(file);

		fprintf_rl(stdout, "Generating the erf_radlim_mid raw LUT took %g sec\n\n", get_time_diff_hr(&ts));
	}
}

void make_erf_radlim_mid_table(const int order, const double step)	// order is <= 6
{
	int32_t i, is, segcount, prec;
	real_t c[COEF_COUNT];
	real_t segstep, segstart, segend, maxseg;
	double start=0., end=4., err, maxerr=0.;
	FILE *file;
	char header_path[64];

	load_erf_radlim_mid_raw_lut();

	segcount = nearbyint(ceil((end-start) / step));

	//sprintf(header_path, "erf_radlim_mid_d%d_step_%g_human.dat", order, step);
	sprintf(header_path, "../rouziclib/math/erf_radlim/mid_value_lut.h");
	file = fopen(header_path, "wb");
	fprintf(file, "{");

	r_init(segstep);
	r_setd(segstep, step);
	r_init(segstart);
	r_init(segend);
	r_init(maxseg);

	r_setd(segend, start);

	for (i=0; i<COEF_COUNT; i++)
		r_init(c[i]);

	for (is=0; is<segcount; is++)
	{
		r_set(segstart, segend);
		r_add(segend, segstep);

		for (i=0; i<COEF_COUNT; i++)
			r_setd(c[i], 0.);

		seg_offset = r_todouble(segstart);

		// Fit
		polynomial_fit_on_function_by_dct_mpfr(f_erf_radlim_mid, segstart, segend, c, order);
		//err = get_polynomial_error_mpfr(f_erf_radlim_mid, segstart, segend, c, order, NEGMODE);

		// Reduce digits
		err = reduce_digits_mpfr(f_erf_radlim_mid, segstart, segend, c, order, NEGMODE, 1.00003, 16.);
		if (err > maxerr)
		{
			maxerr = err;
			r_set(maxseg, segstart);
		}

		// Print to file
		for (i=0; i<order; i++)
			mpfr_fprintf(file, "%.17Rg, ", c[i]);
		mpfr_fprintf(file, "%.17Rg%s", c[order], (is==segcount-1) ? "};\n" : ", \n");
		fflush(file);
	}
	mpfr_fprintf(file, "\nMax error: %g at segment %Rg\n", maxerr, maxseg);
	fprintf(file, "const int order = %d, lutsp = %g;\n", order, nearbyint(1./step));

	fclose (file);

	printf("\nMax error: %g\n", maxerr);
}

// asin

void x_remap(real_t y, real_t x)	// 2.*x - sq(x)
{				// inverse of this is (1. - sqrt(1. - fabs(x))) * sign(x)
	static int init=1;
	static real_t a, b;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(b);
	}

	// 2x
	r_rmuld(a, x, 2.);

	// x^2
	r_sq(b, x);

	r_rsub(y, a, b);
}

void f_asin(real_t y, real_t x)
{
	x_remap(y, x);
	mpfr_asin(y, y, MPFR_RNDN);
}

void make_asin_table_human(const int order, const double step)
{
	int32_t i, is, segcount, prec;
	real_t c[COEF_COUNT];
	real_t segstep, segstart, segend, maxseg;
	double start=0., end=1., err, maxerr=0.;
	FILE *file;
	char header_path[64];

	segcount = nearbyint(ceil(end / step));

	sprintf(header_path, "asin_d%d_step_%g_human.dat", order, step);
	file = fopen(header_path, "wb");

	r_init(segstep);
	r_setd(segstep, step);
	r_init(segstart);
	r_init(segend);
	r_init(maxseg);

	r_setd(segend, start);

	for (i=0; i<COEF_COUNT; i++)
		r_init(c[i]);

	for (is=0; is<segcount; is++)
	{
		r_set(segstart, segend);
		r_add(segend, segstep);

		for (i=0; i<COEF_COUNT; i++)
			r_setd(c[i], 0.);

		// Fit
		polynomial_fit_on_function_by_dct_mpfr(f_asin, segstart, segend, c, order);
		err = get_polynomial_error_mpfr(f_asin, segstart, segend, c, order, NEGMODE);

		// Reduce digits
		err = reduce_digits_mpfr(f_asin, segstart, segend, c, order, NEGMODE, 1.03, 20.);
		if (err > maxerr)
		{
			maxerr = err;
			r_set(maxseg, segstart);
		}

		// Print to file
		mpfr_fprintf(file, "%Rg	", segstart);
		for (i=order; i > 0; i--)
			mpfr_fprintf(file, "% .17Rg   ", c[i]);
		mpfr_fprintf(file, "% .17Rg (err % .3g)\n", c[0], err);
		fflush(file);
	}
	mpfr_fprintf(file, "\nMax error: %g at segment %Rg\n", maxerr, maxseg);

	fclose (file);

	printf("\nMax error: %g\n", maxerr);
}

int main(int argc, char **argv)
{
	mpfr_set_default_prec(PREC);

	//make_erf_radlim_mid_table(3, 1./4.);		// causes SIGSEGV for some reason

	make_cos_double_double_approx();
	test_cos_ddouble();
/*	make_cos_table_human(atoi(argv[1]), 0.01);
	make_cos_table(2, 5, 4);	// 1/2 kB, float err 4.2e-006, double err 6.159e-007
	make_cos_table(3, 5, 4);	// 3 kB, err 1.88958e-009
	make_cos_table(4, 5, 2);	// 2 kB, err 4.63742e-012
	make_cos_table(5, 6, 1);	// 2.5 kB, err ~9e-016, pure err 1.48783e-016
	make_wsinc_table(2, 5, 6.625, 1.6083);
	make_cos_table_human(2, 0.01);

	cos_error_curve();
	//test_matrix();

	if (argc >= 3)
	{
		radlim = atof(argv[1]);
		fprintf_rl(stdout, "radlim %g, step 1/%g\n", radlim, atof(argv[2]));
		make_erf_radlim_table(3, 1./atof(argv[2]));
	}

	if (argc >= 3)
	{
		fprintf_rl(stdout, "degree %d, step 1/%g\n", atoi(argv[1]), atof(argv[2]));
		make_asin_table_human(atoi(argv[1]), 1./atof(argv[2]));
	}*/

	//printf("sizeof(long double) = %d (%d bits). LDBL_DIG  = %d, LDBL_MANT_DIG = %d\n", sizeof(long double), sizeof(long double)*8, LDBL_DIG, LDBL_MANT_DIG);

	return 0;
}
