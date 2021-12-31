#ifndef RL_EXCL_APPROX

uint32_t fastcos_get_param(double *xp, double *endsign, const int quads)
{
	double x=*xp, xoff;
	uint64_t *xint = (uint64_t *) &x;
	uint32_t lutind;

	// x = ]-inf , +inf[ --> x = [0 , 1[
	x = get_fractional_part_positive(x);

	// Quadrant symmetry
	if (quads <= 2)
	{
		// x = [0 , 1[ --> x = [0 , 0.5]
		if (*xint > 0x3FE0000000000000)		// if x > 0.5 when 1 or 2 quadrants
			x = 1. - x;

		// x = [0 , 0.5] --> x = [0 , 0.25]
		if (quads == 1 && *xint > 0x3FD0000000000000)		// if x > 0.25 when 1 quadrant
		{
			*endsign = -1.;
			x = 0.5 - x;
		}
	}

	*xp = x;
	xoff = x + 2.;		// the mantissa for xoff is [1.0 , either 1.125, 1.25 or 1.5]

	lutind = (((uint32_t *) &xoff)[1] & 0x000FFFFF);		// top bits of the mantissa form the LUT index
	return lutind;
}

uint32_t fastcosf_get_param(float *xp)
{
	float x=*xp, xoff;
	uint32_t *xint = (uint32_t *) &x;
	uint32_t lutind;

	// x = ]-inf , +inf[ --> x = [0 , 1]
	x = get_fractional_part_positivef(x);

	*xp = x;
	xoff = x + 2.f;	// the mantissa for xoff is [1.0 , 1.5]

	lutind = (*((uint32_t *) &xoff) & 0x007FFFFF);	// top bits of the mantissa form the LUT index
	return lutind;
}

float fastcosf_tr_d2(float x)	// max error: 2.82e-006 (compare with 1.52017e-007 for cosf())
{
	static const float lut[] = 
	#include "tables/fastcos_d2.h"		// 1 kB
	const uint32_t ish=20-lutsp;
	const float *c;
	uint32_t lutind;

	lutind = fastcosf_get_param(&x) >> ish;
	c = &lut[lutind*3];
	//return fmaf(fmaf(c[2], x, c[1]), x, c[0]);		// too slow
	return (c[2]*x + c[1])*x + c[0];			// less precise
}

double fastcos_tr_d2(double x)	// max error: 6.159e-007
{
	static const double lut[] = 
	#include "tables/fastcos_d2.h"		// 2 kB
	const uint32_t ish=17-lutsp;
	const double *c;
	uint32_t lutind;

	lutind = fastcos_get_param(&x, NULL, quads) >> ish;
	c = &lut[lutind*3];
	//return fma(fma(c[2], x, c[1]), x, c[0]);
	return (c[2]*x + c[1])*x + c[0];
}

double fastcos_tr_d3(double x)	// max error: 1.88958e-009
{
	static const double lut[] = 
	#include "tables/fastcos_d3.h"		// 3 kB
	const uint32_t ish=17-lutsp;
	const double *c;
	uint32_t lutind;

	lutind = fastcos_get_param(&x, NULL, quads) >> ish;
	c = &lut[lutind<<2];
	//return fma(fma(fma(c[3], x, c[2]), x, c[1]), x, c[0]);
	return ((c[3]*x + c[2])*x + c[1])*x + c[0];
}

double fastcos_tr_d4(double x)	// max error: 4.63742e-012
{
	static const double lut[] = 
	#include "tables/fastcos_d4.h"		// 2 kB
	const uint32_t ish=17-lutsp;
	const double *c;
	uint32_t lutind;

	lutind = fastcos_get_param(&x, NULL, quads) >> ish;
	c = &lut[lutind*5];
	//return fma(fma(fma(fma(c[4], x, c[3]), x, c[2]), x, c[1]), x, c[0]);
	return (((c[4]*x + c[3])*x + c[2])*x + c[1])*x + c[0];
}

double fastcos_tr_d5(double x)	// max error: ~9e-016 (compare with 3.41596e-016 for cos())
{
	static const double lut[] = 
	#include "tables/fastcos_d5.h"		// 2.5 kB
	const uint32_t ish=17-lutsp;
	const double *c;
	double endsign = 1.;
	uint32_t lutind;

	lutind = fastcos_get_param(&x, &endsign, quads) >> ish;
	c = &lut[lutind*6];
	return copysign(fma(fma(fma(fma(fma(c[5], x, c[4]), x, c[3]), x, c[2]), x, c[1]), x, c[0]), endsign);
	//return endsign * (((((c[5]*x + c[4])*x + c[3])*x + c[2])*x + c[1])*x + c[0]);
}

xy_t fastcos_tr_d3_xy(xy_t v)
{
	#ifdef RL_INTEL_INTR

	__m128d md = _mm_load_pd((double *) &v);
	md = _mm_fastcos_tr_d3(md);
	_mm_storeu_pd((double *) &v, md);
	return v;

	#endif

	return func1_xy(v, fastcos_tr_d3);
}

#endif

#define COS_Q_CHEB
ddouble_t cos_tr_q(ddouble_t x)	// max error about 4.3e-32 (Chebyshev version, the other is slightly worse)
{
	double endsign = 1.;
	ddouble_t y;
	#ifdef COS_Q_CHEB
	ddouble_t cm[] = {
		{0.47200121576823478, -1.5640151617803393e-17}, 	// T_0 (err 1.56e-34)
		{-0.4994032582704071, 1.4758624598140668e-17},  	// T_1 (err 1.5e-35)
		{0.027992079617547617, 7.3010368448985277e-19}, 	// T_2 (err 4.49e-35)
		{-0.00059669519654884649, -1.3902999147480702e-20},     // T_3 (err 7.07e-38)
		{6.7043948699168399e-06, 2.035995049262044e-22},        // T_4 (err -2.51e-39)
		{-4.6532295897319527e-08, -1.769161868176914e-24},      // T_5 (err -1.36e-40)
		{2.1934576589567331e-10, 2.8463881389941215e-27},       // T_6 (err 1e-43)
		{-7.4816487010336462e-13, 4.1338949231363225e-29},      // T_7 (err -5.31e-46)
		{1.9322978458633277e-15, -7.2354286636588526e-32},      // T_8 (err 2.4e-49)
		{-3.9101701216325904e-18, 6.942454170604681e-35},       // T_9 (err 3.62e-52)
		{6.3670401158338003e-21, 1.579630782651198e-37},        // T_10 (err 5.06e-54)
		{-8.522886041732634e-24, 8.5324378256780345e-41},       // T_11 (err 2.82e-57)
		{9.5446630340576279e-27, 1.1519600914539063e-43},       // T_12 (err 6.84e-61)
		{-9.0744812452201831e-30, -4.2984483792836492e-46},     // T_13 (err 1.25e-63)
		{7.415916419082441e-33, 5.1095287340737115e-49},        // T_14 (err -9.6e-66)
		/* Errors for this implementation based on the maximum degree used:
		   T_12 => 9.11347e-30
		   T_13 => 4.76886e-32
		   T_14 => 4.27417e-32
		   T_15 => 4.27417e-32
		   */
	};
	#else
	ddouble_t c[] = {
		{1, -5.2685651532657577e-36},   			// c0
		{-19.739208802178716, -1.2530591017479423e-15}, 	// c1
		{64.939394022668296, -4.2563201318878282e-15},  	// c2
		{-85.456817206693728, 2.0361752253113143e-16},  	// c3
		{60.244641371876661, -3.2212648404075122e-16},  	// c4
		{-26.426256783374399, 1.1982764379130646e-15},  	// c5
		{7.9035363713184692, -3.9613952660077854e-16},  	// c6
		{-1.714390711088672, -3.4612037008301457e-17},  	// c7
		{0.28200596845579096, 2.1376133124519225e-17},  	// c8
		{-0.036382841142537675, 1.7328614067836044e-18},        // c9
		{0.0037798342004859886, -1.8105199741884246e-20},       // c10
		{-0.0003229910638446016, -1.3130761734743837e-20},      // c11
		{2.3099916358372477e-05, -3.4908602905870041e-22},      // c12
		{-1.4026753595423889e-06, 9.914502332376071e-23},       // c13
		{7.1722342681062607e-08, 4.9416329701476483e-24},       // c14
		/* Errors for this implementation based on the polynomial degree:
		   degree 12 => 9.10454e-30
		   degree 13 => 5.68422e-32
		   degree 14 => 5.17274e-32
		   degree 15 => 5.17274e-32
		   */
	};		
	#endif

	// x = ]-inf , +inf[ --> x = [0 , 1[
	x = sub_qq(x, floor_q(x));

	// Quadrant symmetry
	// x = [0 , 1[ --> x = [0 , 0.5]
	if (cmp_qd(x, 0.5) > 0)
		x = sub_dq(1., x);

	// x = [0 , 0.5] --> x = [0 , 0.25]
	if (cmp_qd(x, 0.25) > 0)
	{
		endsign = -1.;
		x = sub_dq(0.5, x);
	}

	// Square mapping, x = [0 , 0.25] --> x = [0 , 0.0625]
	x = mul_qq(x, x);

	#ifdef COS_Q_CHEB
	// x = [0 , 0.0625] --> x = [-1 , 1] to fit the Chebyshev unit span
	x = sub_qd(mul_qd_simple(x, 32.), 1.);

	// Chebyshev evaluation
	y = eval_chebyshev_polynomial_q(x, cm, 14);

	#else

	// Polynomial evaluation
	y = eval_polynomial_q(x, c, 14);
	#endif

	return mul_qd_simple(y, endsign);
}

ddouble_t cos_q(ddouble_t x)
{
	return cos_tr_q(mul_qq(x, Q_1_2PI));
}
