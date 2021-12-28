// Avoids optimising away certain crucial operations when /fp:fast or -ffast-math are specified
// Works on MSVC and Clang, not on GCC
#ifndef _gcc_
#pragma float_control(push)
#pragma float_control(precise, on)
#endif

ddouble_t ddouble(const double v)
{
	ddouble_t r = {v, 0.};
	return r;
}

// Double to quad basic operations
#ifdef _gcc_
__attribute__((optimize("-fno-fast-math")))
#endif
ddouble_t add_dd_q_quick(double a, double b)
{
	ddouble_t r;
	r.hi = a + b;
	r.lo = b - (r.hi - a);
	return r;
}

#ifdef _gcc_
__attribute__((optimize("-fno-fast-math")))
#endif
ddouble_t add_dd_q(double a, double b)
{
	ddouble_t r;
	r.hi = a + b;
	double v = r.hi - a;
	r.lo = (a - (r.hi - v)) + (b - v);
	return r;
}

#ifdef _gcc_
__attribute__((optimize("-fno-fast-math")))
#endif
ddouble_t sub_dd_q(double a, double b)
{
	ddouble_t r;
	r.hi = a - b;
	double v = r.hi - a;
	r.lo = (a - (r.hi - v)) - (b + v);
	return r;
}

#ifdef _gcc_
__attribute__((optimize("-fno-fast-math")))
#endif
ddouble_t mul_dd_q(double a, double b)
{
	ddouble_t r;
	r.hi = a * b;
	r.lo = fma(a, b, -r.hi);
	return r;
}

// Mixed quad/double operations
ddouble_t add_qd(ddouble_t a, double b)
{
	ddouble_t s = add_dd_q(a.hi, b);
	return add_dd_q_quick(s.hi, a.lo + s.lo);
}

ddouble_t sub_qd(ddouble_t a, double b)
{
	ddouble_t s = sub_dd_q(a.hi, b);
	return add_dd_q_quick(s.hi, a.lo + s.lo);
}

ddouble_t sub_dq(double a, ddouble_t b)
{
	// FIXME probably not ideal
	return add_qd(neg_q(b), a);
}

ddouble_t mul_qd(ddouble_t a, double b)
{
	ddouble_t c = mul_dd_q(a.hi, b);
	return add_dd_q_quick(c.hi, fma(a.lo, b, c.lo));
}

#ifdef _gcc_
__attribute__((optimize("-fno-fast-math")))
#endif
ddouble_t div_qd(ddouble_t a, double b)
{
	double t_hi = a.hi / b;
	ddouble_t p = mul_dd_q(t_hi, b);
	double d_hi = a.hi - p.hi;
	double d_lo = a.lo - p.lo;
	double t_lo = (d_hi + d_lo) / b;
	return add_dd_q_quick(t_hi, t_lo);
}

ddouble_t div_dq(double a, ddouble_t b)
{
	// FIXME probably not ideal
	return mul_qd(recip_q(b), a);
}

// Quad operations
ddouble_t add_qq(ddouble_t a, ddouble_t b)
{
	ddouble_t s = add_dd_q(a.hi, b.hi);
	ddouble_t t = add_dd_q(a.lo, b.lo);
	ddouble_t v = add_dd_q_quick(s.hi, s.lo + t.hi);
	ddouble_t z = add_dd_q_quick(v.hi, t.lo + v.lo);
	return z;
}

ddouble_t sub_qq(ddouble_t a, ddouble_t b)
{
	ddouble_t s = sub_dd_q(a.hi, b.hi);
	ddouble_t t = sub_dd_q(a.lo, b.lo);
	ddouble_t v = add_dd_q_quick(s.hi, s.lo + t.hi);
	ddouble_t z = add_dd_q_quick(v.hi, t.lo + v.lo);
	return z;
}

// Based on https://stackoverflow.com/a/31647953/1675589
#ifdef _gcc_
__attribute__((optimize("-fno-fast-math")))
#endif
ddouble_t mul_qq(ddouble_t a, ddouble_t b)
{
	ddouble_t r, m;
	const double c = 134217729.;
	double up, u1, u2, vp, v1, v2;

	up = a.hi*c;        vp = b.hi*c;
	u1 = (a.hi-up)+up;  v1 = (b.hi-vp)+vp;
	u2 = a.hi-u1;       v2 = b.hi-v1;

	m.hi = a.hi*b.hi;
	m.lo = (((u1*v1-m.hi)+(u1*v2))+(u2*v1))+(u2*v2);

	m.lo += a.hi*b.lo + a.lo*b.hi;
	r.hi = m.hi + m.lo;
	r.lo = m.hi - r.hi + m.lo;

	return r;
}

#ifdef _gcc_
__attribute__((optimize("-fno-fast-math")))
#endif
ddouble_t div_qq(ddouble_t a, ddouble_t b)
{
	double t_hi = a.hi / b.hi;
	ddouble_t r = mul_qd(b, t_hi);
	double pi_hi = a.hi - r.hi;
	double d = pi_hi + (a.lo - r.lo);
	double t_lo = d / b.hi;
	return add_dd_q_quick(t_hi, t_lo);
}

ddouble_t neg_q(ddouble_t a)
{
	ddouble_t r;
	r.hi = -a.hi;
	r.lo = -a.lo;
	return r;
}

#ifdef _gcc_
__attribute__((optimize("-fno-fast-math")))
#endif
ddouble_t recip_q(ddouble_t b)
{
	double t_hi = 1.0 / b.hi;
	ddouble_t r = mul_qd(b, t_hi);
	double pi_hi = 1.0 - r.hi;
	double d = pi_hi - r.lo;
	double t_lo = d / b.hi;
	return add_dd_q_quick(t_hi, t_lo);
}

ddouble_t mul_qd_simple(ddouble_t a, double m)	// multiplier must only change the exponent, not the mantissa
{						// like 0.125, -0.5, 1., -2., 64., ...
	a.hi *= m;
	a.lo *= m;
	return a;
}

int cmp_qq(const ddouble_t *a, const ddouble_t *b)
{
	if (a->hi > b->hi) return 1;
	if (a->hi < b->hi) return -1;
	if (a->lo == b->lo) return 0;
	if (a->lo > b->lo) return 1;
	return -1;
}

int cmp_qd(const ddouble_t a, const double b)
{
	if (a.hi > b) return 1;
	if (a.hi < b) return -1;
	if (a.lo == 0.) return 0;
	if (a.lo > 0.) return 1;
	return -1;
}

ddouble_t floor_q(ddouble_t a)
{
	ddouble_t r;

	r.hi = floor(a.hi);
	r.lo = 0.;

	// If hi is large enough that it is an integer
	if (r.hi == a.hi)
	{
		// Floor lo
		r.lo = floor(a.lo);
		return add_dd_q_quick(r.hi, r.lo);
	}

	return r;
}

#ifndef _gcc_
#pragma float_control(pop)
#endif

#define COS_Q_CHEB
ddouble_t cos_tr_q(ddouble_t x)	// max error about 4.2e-32 (Chebyshev version, the other is slightly worse)
{
	double endsign = 1.;
	ddouble_t y;
	#ifdef COS_Q_CHEB
	ddouble_t cm[] = {
		{0.47200121576823478, -1.5640151617803393e-17}, 	// T_0 (err 1.56e-34)
		{-0.4994032582704071, 1.4758624598140668e-17},  	// T_2 (err 1.5e-35)
		{0.027992079617547617, 7.3010368448985277e-19}, 	// T_4 (err 4.49e-35)
		{-0.00059669519654884649, -1.3902999147480702e-20},     // T_6 (err 7.07e-38)
		{6.7043948699168399e-06, 2.035995049262044e-22},        // T_8 (err -2.51e-39)
		{-4.6532295897319527e-08, -1.769161868176914e-24},      // T_10 (err -1.36e-40)
		{2.1934576589567331e-10, 2.8463881389941215e-27},       // T_12 (err 1e-43)
		{-7.4816487010336462e-13, 4.1338949231363225e-29},      // T_14 (err -5.31e-46)
		{1.9322978458633277e-15, -7.2354286636588526e-32},      // T_16 (err 2.4e-49)
		{-3.9101701216325904e-18, 6.942454170604681e-35},       // T_18 (err 3.62e-52)
		{6.3670401158338003e-21, 1.579630782651198e-37},        // T_20 (err 5.06e-54)
		{-8.522886041732634e-24, 8.5324378256780345e-41},       // T_22 (err 2.82e-57)
		{9.5446630340576279e-27, 1.1519600914539063e-43},       // T_24 (err 6.84e-61)
		{-9.0744812452201831e-30, -4.2984483792836492e-46},     // T_26 (err 1.25e-63)
		{7.415916419082441e-33, 5.1095287340737115e-49},        // T_28 (err -9.6e-66)
		/* Errors for this implementation based on the maximum degree used:
		   T_24 => 9.10396e-30
		   T_26 => 4.26725e-32
		   T_28 => 4.16839e-32
		   T_30 => 4.16839e-32
		   */
	};
	#else
	ddouble_t c[] = {
		{1, -5.2685651532657577e-36},   			// c0
		{-19.739208802178716, -1.2530591017479423e-15}, 	// c2
		{64.939394022668296, -4.2563201318878282e-15},  	// c4
		{-85.456817206693728, 2.0361752253113143e-16},  	// c6
		{60.244641371876661, -3.2212648404075122e-16},  	// c8
		{-26.426256783374399, 1.1982764379130646e-15},  	// c10
		{7.9035363713184692, -3.9613952660077854e-16},  	// c12
		{-1.714390711088672, -3.4612037008301457e-17},  	// c14
		{0.28200596845579096, 2.1376133124519225e-17},  	// c16
		{-0.036382841142537675, 1.7328614067836044e-18},        // c18
		{0.0037798342004859886, -1.8105199741884246e-20},       // c20
		{-0.0003229910638446016, -1.3130761734743837e-20},      // c22
		{2.3099916358372477e-05, -3.4908602905870041e-22},      // c24
		{-1.4026753595423889e-06, 9.914502332376071e-23},       // c26
		{7.1722342681062607e-08, 4.9416329701476483e-24},       // c28
		/* Errors for this implementation based on the maximum degree used:
		   degree 24 => 9.10454e-30
		   degree 26 => 5.68422e-32
		   degree 28 => 5.17274e-32
		   degree 30 => 5.17274e-32
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

	#ifdef COS_Q_CHEB
	// x = [0 , 0.25] --> x = [0 , 1] to fit the Chebyshev unit span
	x = mul_qd_simple(x, 4.);

	// Chebyshev evaluation
	y = eval_chebyshev_polynomial_even_q(x, cm, 28);

	#else

	// Polynomial evaluation
	y = eval_polynomial_q(mul_qq(x, x), c, 28/2);
	#endif

	return mul_qd_simple(y, endsign);
}
