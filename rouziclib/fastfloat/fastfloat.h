/*
	Summary of the main trigonometry functions:

			approx	builtin		LUT	builtin
			speed	speed	error	size	equivalent
			----------------------------------------------------------------------------------------------------------
	fastlog2(x)	~13	~49	7.1e-9	3 kB	log2(x), except fastlog2(x) never returns a NaN or inf
	fastexp2(x)	~31	~205	6.7e-9	1.5 kB	exp2(x), only valid if x = [-1022.0 , 1024.0[
	fastpow(x,y)	~63	~450	2.6e-8		pow(x,y), returns bogus results instead of NaNs or inf
	fastsqrt(x)	~21.5	~12.5	9e-10	3 kB	sqrt(x), disregards the sign of x
 */


#ifndef FASTFLOAT_H
#define FASTFLOAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
	
#ifdef _MSC_VER
	#define INLINE __forceinline
#else
	#define INLINE inline
#endif

// log2 approximation, returns slightly bogus values for 0, subnormals, inf and NaNs, returns the real value only for negative x
// max error of 7.098e-09 for lutsp of 7, lut takes 3 kB
static INLINE double fastlog2(double x)
{
	static const double lut[] = 
	#include "fastlog2.h"
	uint64_t xint, m, m1int;
	double m1, mlog;
	int32_t exp, lutind;
	const int64_t ish = 52-lutsp;		// index shift
	const double *c;
	double c0, c1, c2;

	xint = *((uint64_t *) &x);
	exp = (xint & 0x7FF0000000000000) >> 52;	// exponent mask and shift

	exp -= 1023;			// exponent offset compensation

	m = xint & 0x000FFFFFFFFFFFFF;	// 52-bit mantissa
	m1int = 0x3FF0000000000000 | m;	// 1.0 + mantissa
	m1 = *((double *) &m1int);	// m1 = [1.0 , 2.0[

	lutind = m >> ish;

	c = &lut[lutind*3];    c0 = c[0];    c1 = c[1];    c2 = c[2];
	mlog = (c2*m1 + c1)*m1 + c0;	// logarithm of the mantissa

	return (double) exp + mlog;
}

// exp2 approximation, returns bogus results if x is outside [-1022.0 , 1024.0[
// max error of 6.65255e-09 for lutsp of 6, lut takes 1.5 kB
static INLINE double fastexp2(double x)
{
	static const double lut[] = 
	#include "fastexp2.h"
	double xr, xf, xf1, y, xfe;
	int32_t exp, lutind;
	const int64_t ish = 52-lutsp;		// index shift
	uint64_t yint, m;
	const double *c;
	double c0, c1, c2;

	xr = floor(x);	// real part of x
	xf = x - xr;	// fractional part of x. xf = [0.0 , 1.0[
	xf1 = xf + 1.;

	exp = xr;
	exp += 1023;	// IEEE-754 exponent. Will be bogus if x < -1022 or x >= 1024

	yint = (uint64_t) exp << 52;			// put the exponent in the final result
	y = *((double *) &yint);			// now y = 2^xr

	m = *((uint64_t *) &xf1) & 0x000FFFFFFFFFFFFF;	// 52-bit mantissa of the fractional part
	lutind = m >> ish;

	c = &lut[lutind*3];    c0 = c[0];    c1 = c[1];    c2 = c[2];
	xfe = (c2*xf + c1)*xf + c0;			// 2^xf

	return y * xfe;					// y = 2^xr * 2^xf
}

// max error is 2.627e-08
static INLINE double fastpow(double x, double y)
{
	return fastexp2(fastlog2(x) * y);
}

// Oops, it's actually slower than sqrt(x)
static INLINE double fastsqrt(double x)
{
	static const double lut[] = 
	#include "fastsqrt.h"
	uint64_t xint, m, m1int, explsbint;
	double m1, ms, expmul, es;
	int32_t lutind;
	int64_t exp;
	const double ec = sqrt(0.5) - 0.5;
	const int64_t ish = 52-lutsp;		// index shift
	const double *c;
	double c0, c1, c2;

	xint = *((uint64_t *) &x);
	exp = (xint & 0x7FF0000000000000) >> 52;	// exponent mask and shift
	exp -= 1023;					// exponent offset compensation

	explsbint = (exp & 0x1) << 62;		// least significant bit of the exponent promoted to most signficant bit
	expmul = *((double *) &explsbint);	// expmul is now either 0.0 or 2.0
	expmul = 1. + expmul * ec;		// expmul is now either 1.0 or 1.4142...

	exp >>= 1;			// exponent of the sqrt of x is half the exponent of x
	exp += 1023;			// exponent offset compensation
	exp <<= 52;			// put the exponent back in its IEEE double place
	es = *((double *) &exp);	// sqrt of the exponent

	m = xint & 0x000FFFFFFFFFFFFF;	// 52-bit mantissa
	m1int = 0x3FF0000000000000 | m;	// 1.0 + mantissa
	m1 = *((double *) &m1int);	// m1 = [1.0 , 2.0[

	lutind = m >> ish;

	c = &lut[lutind*3];    c0 = c[0];    c1 = c[1];    c2 = c[2];
	ms = (c2*m1 + c1)*m1 + c0;	// sqrt of the mantissa

	return es * expmul * ms;
}

static INLINE double fastatan2(double y, double x)	// as it is the caller must provide numbers large enough to be treated as integers
{
	double th;
	int32_t xi, yi;
	const double convratio = 2.*pi / 4294967296.;

	xi = roundaway(x);
	yi = roundaway(y);

	th = fpatan2(yi, xi);
	th *= convratio;

	return th;
}

static INLINE double fastgaussian(double x)
{
	int32_t xi;
	double y;
	const double convratio = 1. / 1073741824.;

	xi = roundaway(x*65536.);

	y = fpgauss_d1i(xi);

	y *= convratio;

	return y;
}

static INLINE double fasterfr(double x)
{
	int32_t xi;
	double y;
	const double convratio = 1. / 1073741824.;

	xi = roundaway(x*65536.);

	y = fperfr_d1i(xi);

	y *= convratio;

	return y;
}

#ifdef __cplusplus
}
#endif
#endif
