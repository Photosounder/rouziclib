#ifndef RL_EXCL_APPROX

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

// log2 approximation, returns slightly bogus values for 0, subnormals, inf and NaNs, returns the real value only for negative x
// max error of 7.098e-09 for lutsp of 7, lut takes 3 kB
double fastlog2(double x)
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
double fastexp2(double x)
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
double fastpow(double x, double y)
{
	return fastexp2(fastlog2(x) * y);
}

// Oops, it's actually slower than sqrt(x)
double fastsqrt(double x)
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

/*__float128 fastcosq(__float128 x)
{
	static const __float128 lut[] = 
	#include "fastcos_d5.h"
	const __float128 *c;
	const uint32_t ish=15-lutsp;
	const __float128 inv_2pi = 0.159154943091895335768883763372514362034Q;
	__float128 endsign = 1.Q;
	__float128 xfloor = 0.Q;
	__float128 xoff;
	uint64_t *xhi8 = &((uint64_t *) &x)[1];		// top 8 bytes of x
	uint32_t *xhi4 = &((uint32_t *) &x)[3];		// top 4 bytes of x
	uint64_t *xfhi = &((uint64_t *) &xfloor)[1];	// top 8 bytes of xfloor
	uint64_t *xflo = &((uint64_t *) &xfloor)[0];	// bottom 8 bytes of xfloor
	uint32_t exp, lutind;
	uint64_t xfmask;

	x *= inv_2pi;				// convert from radians to turns
	*xhi8 &= 0x7FFFFFFFFFFFFFFF;		// x = |x|

	// x = [0 , +inf[ --> x = [0 , 1[
	if (*xhi4 >= 0x3FFF0000)		// if |x| >= 1.0
	{
		exp = (*xhi4 >> 16) - 16383;

		if (exp <= 48)
		{
			xfmask = 0xFFFFFFFFFFFFFFFF << (48-exp);	// mask to exclude bits of the mantissa that are fractional
			*xfhi = *xhi8 & xfmask;
		}
		else
		{
			xfloor = x;
			xfmask = 0xFFFFFFFFFFFFFFFF << (112-exp);	// mask to exclude bits of the mantissa that are fractional
			*xflo &= xfmask;
		}

		x -= xfloor;						// |x| = fractional part of |x|
	}

	// x = [0 , 1[ --> x = [0 , 0.5]
	if (*xhi4 > 0x3FFE0000)		// if x > 0.5
		x = 1.Q - x;

	// x = [0 , 0.5] --> x = [0 , 0.25]
	if (*xhi4 > 0x3FFD0000)		// if x > 0.25
	{
		endsign = -1.Q;
		x = 0.5Q - x;
	}

	xoff = x + 0.5Q;	// the mantissa for xoff is [1.0 , 1.5]

	lutind = (((uint32_t *) &xoff)[3] & 0x0000FFFF) >> ish;		// top bits of the mantissa form the LUT index
	c = &lut[lutind*6];
	return endsign * (((((c[5]*x + c[4])*x + c[3])*x + c[2])*x + c[1])*x + c[0]);
}*/

uint32_t fastcos_get_param(double *xp, double *endsign)
{
	const double inv_2pi = 0.15915494309189533576888;
	double x, xoff, xfloor = 0.;
	uint64_t *xint = (uint64_t *) &x;		// top 8 bytes of x
	uint32_t *xhi4 = &((uint32_t *) &x)[1];		// top 4 bytes of x
	uint64_t *xfint = (uint64_t *) &xfloor;		// top 8 bytes of xfloor
	uint32_t exp, lutind;
	uint64_t xfmask;

	x = *xp * inv_2pi;			// convert from radians to turns
	*xint &= 0x7FFFFFFFFFFFFFFF;		// x = |x|

	// x = [0 , +inf[ --> x = [0 , 1[
	if (*xhi4 >= 0x3FF00000)		// if |x| >= 1.0
	{
		exp = (*xhi4 >> 20) - 1023;

		xfmask = 0xFFFFFFFFFFFFFFFF << (52-exp);	// mask to exclude bits of the mantissa that are fractional
		*xfint = *xint & xfmask;

		x -= xfloor;					// |x| = fractional part of |x|
	}

	// x = [0 , 1[ --> x = [0 , 0.5]
	if (*xhi4 > 0x3FE00000)		// if x > 0.5
		x = 1. - x;

	// x = [0 , 0.5] --> x = [0 , 0.25]
	if (*xhi4 > 0x3FD00000)		// if x > 0.25
	{
		*endsign = -1.;
		x = 0.5 - x;
	}

	*xp = x;
	xoff = x + 0.5;		// the mantissa for xoff is [1.0 , 1.5]

	lutind = (((uint32_t *) &xoff)[1] & 0x000FFFFF);		// top bits of the mantissa form the LUT index
	return lutind;
}

uint32_t fastcosf_get_param(float *xp, float *endsign)
{
	const float inv_2pi = 0.159154943f;
	float x, xoff, xfloor = 0.f;
	uint32_t *xint = (uint32_t *) &x;
	uint32_t *xfint = (uint32_t *) &xfloor;
	uint32_t exp, lutind, xfmask;

	x = *xp * inv_2pi;		// convert from radians to turns
	*xint &= 0x7FFFFFFF;		// x = |x|

	// x = [0 , +inf[ --> x = [0 , 1[
	if (*xint >= 0x3F800000)		// if |x| >= 1.0
	{
		exp = (*xint >> 23) - 127;

		xfmask = 0xFFFFFFFF << (23-exp);	// mask to exclude bits of the mantissa that are fractional
		*xfint = *xint & xfmask;

		x -= xfloor;				// |x| = fractional part of |x|
	}

	// x = [0 , 1[ --> x = [0 , 0.5]
	if (*xint > 0x3F000000)		// if x > 0.5
		x = 1.f - x;

	// x = [0 , 0.5] --> x = [0 , 0.25]
	if (*xint > 0x3E800000)		// if x > 0.25
	{
		*endsign = -1.f;
		x = 0.5f - x;
	}

	*xp = x;
	xoff = x + 0.5f;	// the mantissa for xoff is [1.0 , 1.5]

	lutind = (*((uint32_t *) &xoff) & 0x007FFFFF);	// top bits of the mantissa form the LUT index
	return lutind;
}

float fastcosf_d2(float x)	// max error: 4.79042e-007 (compare with 1.52017e-007 for cosf())
{
	static const float lut[] = 
	#include "fastcos_d2.h"		// 780 bytes
	const uint32_t ish=22-lutsp;
	const float *c;
	float endsign = 1.f;
	uint32_t lutind;

	lutind = fastcosf_get_param(&x, &endsign) >> ish;
	c = &lut[lutind*3];
	return endsign * ((c[2]*x + c[1])*x + c[0]);
}

double fastcos_d2(double x)	// max error: 7.70006e-008
{
	static const double lut[] = 
	#include "fastcos_d2.h"		// 1560 bytes
	const uint32_t ish=19-lutsp;
	const double *c;
	double endsign = 1.;
	uint32_t lutind;

	lutind = fastcos_get_param(&x, &endsign) >> ish;
	c = &lut[lutind*3];
	return endsign * ((c[2]*x + c[1])*x + c[0]);
}

double fastcos_d5(double x)	// max error: 9.62572e-015 (compare with 3.41596e-016 for cos())
{
	static const double lut[] = 
	#include "fastcos_d5.h"		// 1584 bytes
	const uint32_t ish=19-lutsp;
	const double *c;
	double endsign = 1.;
	uint32_t lutind;

	lutind = fastcos_get_param(&x, &endsign) >> ish;
	c = &lut[lutind*6];
	return endsign * (((((c[5]*x + c[4])*x + c[3])*x + c[2])*x + c[1])*x + c[0]);
}

float fastgaussianf_d0(float x)
{
	#include "fastgauss_d0.h"	// contains the LUT, offset and limit
	uint32_t index, *xint = (uint32_t *) &x;

	*xint &= 0x7FFFFFFF;		// x = |x|

	if (x > limit)			// if x isn't represented in the LUT
		return 0.f;

	x += offset;			// turns the mantissa into an integer suitable as a LUT index
	index = *xint & 0x007FFFFF;	// keeps mantissa
	return fastgauss_lut[index];
}

float fastgaussianf_d1(float x)
{
	#include "fastgauss_d1.h"	// contains the LUT, offset and limit
	uint32_t index, *xint = (uint32_t *) &x;
	float xa;
	const float *c;

	*xint &= 0x7FFFFFFF;		// x = |x|
	xa = x;

	if (x > limit)			// if x isn't represented in the LUT
		return 0.f;

	x += offset;			// turns the mantissa into an integer suitable as a LUT index
	index = *xint & 0x007FFFFE;	// keeps mantissa
	c = &fastgauss_lut[index];
	return c[1]*xa + c[0];
}

float fasterfrf_d0(float x)
{
	#include "fasterfr_d0.h"	// contains the LUT, offset and limit
	uint32_t index, *xint = (uint32_t *) &x;

	if (x > limit)			// if x isn't represented in the LUT
		return 1.f;
	if (x < -limit)			// if x isn't represented in the LUT
		return 0.f;

	x += offset;			// turns the mantissa into an integer suitable as a LUT index
	index = *xint & 0x007FFFFF;	// keeps mantissa
	return fasterfr_lut[index];
}

float fasterfrf_d1(float x)
{
	#include "fasterfr_d1.h"	// contains the LUT, offset and limit
	uint32_t index, *xint = (uint32_t *) &x;
	float xa;
	const float *c;

	if (x > limit)			// if x isn't represented in the LUT
		return 1.f;
	if (x < -limit)			// if x isn't represented in the LUT
		return 0.f;

	xa = x;

	x += offset;			// turns the mantissa into an integer suitable as a LUT index
	index = *xint & 0x007FFFFE;	// keeps mantissa
	c = &fasterfr_lut[index];
	return c[1]*xa + c[0];
}

// The following functions just wrap to the matching fixed-point function
double fastatan2(double y, double x)	// as it is the caller must provide numbers large enough to be treated as integers
{
	double th;
	int32_t xi, yi;
	const double convratio = 2.*3.1415926535897932 / 4294967296.;

	xi = roundaway(x);
	yi = roundaway(y);

	th = fpatan2_d2(yi, xi);
	th *= convratio;

	return th;
}

#endif
