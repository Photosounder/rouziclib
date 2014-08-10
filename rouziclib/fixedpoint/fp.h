/*
	Summary of the main trigonometry functions:

			format	format	new	float	float
			in	out	speed	speed	equivalent
			----------------------------------------------------------------------------------------------------------
	isqrt(x)	u64	u32	49.25	64.5	sqrt(x)
	fplog2(x)	32.0	s5.26	31.33	66	log2(x), fplog2(0) returns 0 instead of a NaN
	fpexp2(x)	s5.26	32.0	12.66	142	exp2(x) or 2^x, returns 0 is x is negative as if it was 2^-inf
	fppow(x,y,fmt)	?,s5.26	.fmt	59	3.6-12k	pow(x, y) or x^y, x > 0 and in a custom (32-fmt).fmt format
	fpcos(x)	0.32	s1.30	5.5	94	cos(2.*pi*x), input unit is in turns, not radians
	fpwsinc(x)	s2+.24	s1.30	5?	?	Blackman-windowed sinc with a cutoff frequency of 0.5 and rolloff bandwidth of 0.5
	fpatan2(y, x)	s31,s31	s0.32	25-70	155	atan2(y, x)/2pi (output unit is turns, not radians), fpatan2(y, x) = [-0.5, +0.5[
	fpgauss(x)	s3+.16	0.30	5?	144?	exp(-x*x) for x = [-4.0, 4.0], returns 0.0 outside of that range
	fperfr(x)	s3+.16	0.30	5?	?	0.5erf(x)+0.5 for x = [-4.0, 4.0], returns 0.0 or 1.0 outside of that range
 */


#ifndef FP_H
#define FP_H

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

#define QF	16	// Q format fractional bit count
/*#define ONE	65536
#define ONEF	65536.f*/

// accurate, ~1.5 cycles instead of ~5, sometimes the compiler already does this
static INLINE int32_t fastabs(int32_t x)
{
	int32_t y = x >> 31;
	return (x^y) - y;
}

static INLINE int32_t fp_div(int64_t a, int32_t b)
{
	return (a << QF) / b;
}

static INLINE int32_t linear_interpolation(int32_t a, int32_t b, int32_t t)	// t must be in 1.QF format
{
	return (((int64_t) (b-a) * t) >> QF) + a;	// the 64-bit cast is required for the multiplication depending on the FP format
}

// UNUSED
static INLINE int32_t ones32(uint32_t x)	// gives you the number of set bits
{
	x -= ((x >> 1) & 0x55555555);
	x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
	x = (((x >> 4) + x) & 0x0f0f0f0f);
	x += (x >> 8);
	x += (x >> 16);
	return (x & 0x0000003f);
}

static INLINE int32_t log2_int_even(uint32_t x)	// returns the number of bits up to the most significant set bit so that 2^return > x >= 2^(return-1)
{
	// table could be uint8_t but it's half a cycle faster in 32 bits
	static int32_t table[64] = { 0,  2, -1, 16, -1,  2, 30, -1, 18, -1, -1, -1,  4, 22, 30, -1, -1, -1 , 20, 18, 12, -1, 14, -1, -1,  4, -1,  8, -1, 24, 32, -1, 16, -1, 28, -1 , -1, -1, 22, -1, 20, 10, 12, -1,  6, -1, -1, 14, 28, -1, -1, 10, -1, 6, -1, 26, -1,  8, 26, -1, 24, -1, 32, -1 };	// this version of the table returns upper even values for the purposes of the isqrt() function

     	x |= (x >> 1);		// Propagate leftmost
	x |= (x >> 2);		// 1-bit to the right.
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x *= 0x06EB14F9;	// Multiplier is 7*255**3.
	return table[x >> 26];
}

static INLINE int32_t log2_int(uint32_t x)	// returns the number of bits up to the most significant set bit so that 2^return > x >= 2^(return-1)
{
	// table could be uint8_t but it's half a cycle faster in 32 bits
	static int32_t table[64] = { 0,  1, -1, 16, -1,  2, 29, -1, 17, -1, -1, -1,  3, 22, 30, -1, -1, -1 , 20, 18, 11, -1, 13, -1, -1,  4, -1,  7, -1, 23, 31, -1, 15, -1, 28, -1 , -1, -1, 21, -1, 19, 10, 12, -1,  6, -1, -1, 14, 27, -1, -1,  9, -1, 5, -1, 26, -1,  8, 25, -1, 24, -1, 32, -1 };

     	x |= (x >> 1);		// Propagate leftmost
	x |= (x >> 2);		// 1-bit to the right.
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x *= 0x06EB14F9;	// Multiplier is 7*255**3.
	return table[x >> 26];
}

// (for size 10) accurate to either <= 10 integer units or 1 in 1.9 M (sqrtf() is about 18x more accurate, 1 in 34M)
// takes 49.25 cycles as opposed to 64.5 for sqrtf()
static INLINE uint32_t isqrt(uint64_t x)
{
	#include "fracsqrt.h"	// includes the LUT, the entry bit precision (prec), LUT size power (lutsp) and how many max bits |b-a| takes (abdp)
	const uint32_t ish=32-lutsp, ip=32-prec, tp=32+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t xt, xb, expon, frx, lutind, t, a, b, fs;
	uint64_t frxl;

	xt = x >> 32;	// top half of x

	if (xt)		// if x >= 2^32
	{
		expon = log2_int_even(xt) + 32;
		frxl = x << (64-expon);
		frx = frxl >> 32;
	}
	else
	{
		xb = x;				// bottom half of x
		expon = log2_int_even(xb);	// x = frx * 2^expon	->	isqrt(x) = sqrt(frx) * 2^(expon/2)
		frx = xb << (32-expon);		// turns the fractional part of normalised x into a 0.32 fp value
	}

	lutind = frx >> ish;			// index for the LUT, 12 bits
	t = (frx & tmask) >> tbd;		// interpolator betweem two LUT entries, 16 bits

	a = fracsqrt[lutind];
	b = fracsqrt[lutind+1];
	fs = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);		// fs = sqrt(frx) by linear interpolation of a and b by t in 0.32 format

	return fs >> (32-(expon>>1));		// truncates the result
	
	/*fs >>= 31-(expon>>1);			// rounds off the result
	fs += fs & 1;				// but costs 5 extra cycles
	fs >>= 1;				// which isn't really worth the extra precision
	return fs;*/
}

// takes 31.33 cycles as opposed to 66 cycles for log2f() or logf()
// treats x as an integer, returns result in s5.26 format within the range [0.0 , 32.0[, except for fplog2(0) = undefined
static INLINE int32_t fplog2(uint32_t x)		// input is in 32.0, output is in s5.26
{
	#include "fraclog2.h"	// includes LUT, the entry bit precision (prec), LUT size power (lutsp) and how many max bits |b-a| takes (abdp)
	const uint32_t ish=32-lutsp, ip=31-prec, tp=31+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, expon, frx, t;
	int32_t a, b, fl;

	expon = log2_int(x);			// x = frx * 2^expon	->	fplog2(x) = log2(frx) + expon
	frx = x << (32-expon);			// turns the fractional part of normalised x into a 0.32 fp value
//	frx >>= (32-26);			// turns it into a 0.26 fp value

	lutind = frx >> ish;			// index for the LUT
	t = (frx & tmask) >> tbd;		// interpolator betweem two LUT entries

	a = fraclog2[lutind];
	b = fraclog2[lutind+1];
	fl = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);	// fl = log2(frx) by linear interpolation of a and b by t in 0.31 format

	return (expon<<26) + (fl>>5);		// output format is s5.26
}

// takes 12.66 cycles as opposed to 142 cycles for exp2f()
static INLINE uint32_t fpexp2(int32_t x)	// input is s5.26, output is in 32.0
{
	#include "fracexp2.h"	// includes the fractional 2^x table as generated by tablegen.exe, the entry bit precision (prec) and LUT size power (lutsp)
	const uint32_t ish=26-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, expon, frx, t;
	uint32_t a, b, fe;

	if (x < 0)		// treat negative input as being like -infinity
		return 0;

	expon = x>>26;			// extract the exponent, 5.0 fp value
	frx = x & 0x3FFFFFF;		// the fractional part is the 26 lower bits, 0.26 fp value

	lutind = frx >> ish;			// index for the LUT
	t = (frx & tmask) >> tbd;		// interpolator betweem two LUT entries

	a = fracexp2[lutind];
	b = fracexp2[lutind+1];
	fe = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);		// fe = exp2(frx) by linear interpolation of a and b by t in 2.30 format

	if (expon > 30)
		return (fe<<(expon-30));
	else
		return (fe>>(30-expon));		// output format is 32.0
}

static INLINE int32_t fppow(uint32_t x, int32_t y, const uint32_t fmt)	// does pow(x, y) or x^y by doing exp2(log2(x) * y) or 2^(y*log2(x))
{
	int32_t xlog, xyl, r;

	xlog = fplog2(x) - (fmt<<26);			// log2(x) with x in (32-fmt).fmt format (for instance if fmt is 24 then x input format is 8.24, xlog stays in s5.26)
	xyl = ((uint64_t) xlog * (uint64_t) y) >> 26;	// doesn't handle overflows very gracefully as it might go up to s10.26
	r = fpexp2(xyl+(fmt<<26));			// in (32-fmt).fmt format

	return r;
}

#define fpsin(x) fpcos((x)+0xC0000000)

// takes 5.5 cycles (+0.66 cycles if abdp!=0) compared to 94 cycles for cosf()
static INLINE int32_t fpcos(uint32_t x)		// does the equivalent of cos(2.*pi*x), x = [0.0 , 1.0[ = [0 , 2^32[ = 0.32 fp value
{
	#include "fpcos.h"	// includes the cos LUT as generated by tablegen.exe, the entry bit precision (prec) and LUT size power (lutsp)
	const uint32_t outfmt = 30;	// final output format in s1.outfmt
	const uint32_t ofs=30-outfmt, ish=32-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, t;
	int32_t a, b, y;

	lutind = x >> ish;		// index for the LUT
	t = (x & tmask) >> tbd;		// interpolator betweem two LUT entries

	a = fpcos_lut[lutind];
	b = fpcos_lut[lutind+1];
	y = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);	// y = cos(x) by linear interpolation of a and b by t in s1.30 format

	return y >> ofs;		// truncates the result, output in s1.outfmt format
}

// accurate to 0.07667 arc seconds for size 10 (atan2f() is about 6x more accurate)
// takes presumably 25 cycles in 64-bit, 70 cycles in 32-bit (the 64-bit division kills it, takes 17 cycles without it). atan2f() takes about 155 cycles
static INLINE int32_t fpatan2(int32_t y, int32_t x)		// does the equivalent of atan2(y, x)/2pi, y and x are integers, not fixed point
{
	#include "fpatan.h"	// includes the atan LUT as generated by tablegen.exe, the entry bit precision (prec) and LUT size power (lutsp)
	const uint32_t outfmt = 32;	// final output format in s0.outfmt
	const uint32_t ofs=30-outfmt, ds=29, ish=ds-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// ds is the division shift, the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	const uint32_t halfof = 1UL<<(outfmt-1);	// represents 0.5 in the ouput format, which since it is in turns means half a circle
	const uint32_t pds=ds-lutsp;	// division shift and post-division shift
	uint32_t lutind, p, t, d;
	int32_t a, b, xa, ya, xs, ys, div, r;

	xs = x >> 31;		// equivalent of fabs()
	xa = (x^xs) - xs;
	ys = y >> 31;
	ya = (y^ys) - ys;

	d = ya+xa;
	if (d==0)		// if both y and x are 0 then they add up to 0 and we must return 0
		return 0;

	// the following does 0.5 * (1. - (y-x) / (y+x))
	// (y+x) is u1.31, (y-x) is s0.31, div is in s1.29

	div = ((int64_t) (ya-xa)<<ds) / d;	// '/d' normalises distance to the unit diamond, immediate result of division is always <= +/-1^ds
	p = ((1UL<<ds) - div) >> 1;		// before shift the format is s2.29. position in u1.29

	lutind = p >> ish;		// index for the LUT, 12 bits
	t = (p & tmask) >> tbd;		// interpolator betweem two LUT entries, 16 bits

	a = fpatan_lut[lutind];
	b = fpatan_lut[lutind+1];
	r = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);	// y = cos(x) by linear interpolation of a and b by t in s1.30 format
//	r = (r>>(2+ofs));		// r is divided by 4 because the LUT's entries are in a x4 format, then put it in s0.outfmt format
//	disregard that, r is s0.32, which is possible because the top non-sign bit is unused

	// Quadrants
	if (xs)				// if x was negative
		r = halfof - r;		// r = 0.5 - r

	r = (r^ys) - ys;		// if y was negative then r is negated

	return r;
}

static INLINE int32_t fpwsinc(int32_t x)	// Windowed sinc function in the [-2 , 2] range, input is 2+.24, output is s1.30
{
	#include "fpwsinc.h"	// includes the one-sided gaussian LUT as generated by tablegen.exe, the entry bit precision (prec) and LUT size power (lutsp)
	const uint32_t outfmt = 30;	// final output format in s1.outfmt
	const uint32_t ofs=30-outfmt, ish=25-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, t;
	int32_t a, b, y;

	x = fastabs(x);

	if (x >= 2<<24)
		return 0;

	lutind = x >> ish;		// index for the LUT
	t = (x & tmask) >> tbd;		// interpolator betweem two LUT entries

	a = fpwsinc_lut[lutind];
	b = fpwsinc_lut[lutind+1];
	y = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);

	return y >> ofs;		// truncates the result, output in s1.outfmt format
}

static INLINE int32_t fpgauss(int32_t x)	// Gaussian function in the [-4 , 4] range, input is 3+.16, output is 1.30
{
	#include "fpgauss.h"	// includes the one-sided gaussian LUT as generated by tablegen.exe, the entry bit precision (prec) and LUT size power (lutsp)
	const uint32_t outfmt = 30;	// final output format in 1.outfmt
	const uint32_t ofs=30-outfmt, ish=18-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, t;
	int32_t a, b, y;

	x = fastabs(x);

	if (x >= 4<<16)
		return 0;

	lutind = x >> ish;		// index for the LUT
	t = (x & tmask) >> tbd;		// interpolator betweem two LUT entries

	a = fpgauss_lut[lutind];
	b = fpgauss_lut[lutind+1];
	y = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);

	return y >> ofs;		// truncates the result, output in s1.outfmt format
}

static INLINE int32_t fperfr(int32_t x)	// 0.5erf(x)+0.5 function in the [-4 , 4] range, input is s3+.16, output is 1.30
{
	#include "fperfr.h"	// includes the one-sided gaussian LUT as generated by tablegen.exe, the entry bit precision (prec) and LUT size power (lutsp)
	const uint32_t outfmt = 30;	// final output format in 1.outfmt
	const uint32_t ofs=30-outfmt, ish=19-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, t;
	int32_t a, b, y;

	if (x >= 4<<16)
		return 1<<outfmt;
	if (x <= -4<<16)
		return 0;

	x += 4<<16;

	lutind = x >> ish;		// index for the LUT
	t = (x & tmask) >> tbd;		// interpolator betweem two LUT entries

	a = fperfr_lut[lutind];
	b = fperfr_lut[lutind+1];
	y = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);

	return y >> ofs;		// truncates the result, output in s1.outfmt format
}

#ifdef __cplusplus
}
#endif
#endif
