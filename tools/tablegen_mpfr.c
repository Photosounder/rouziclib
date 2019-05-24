//	gcc tablegen_mpfr.c -o tablegen_mpfr.exe -std=c99 -lm -lmpfr -lgmp -lwinmm -w -O0 && ./tablegen_mpfr

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <windows.h>

#define RL_MPFR
#include "../rouziclib/rouziclib.h"
#include "../rouziclib/rouziclib.c"

#define PREC	256

#define COEF_COUNT 7

void diff_f(real dft, real x, void (*f)(real,real), real *c)
{
	//return f(x) - (((((c5*x + c4)*x + c3)*x + c2)*x + c1)*x + c0);

	int ic;
	static int init=1;
	static real a;

	if (init)
	{
		init = 0;
		r_init(a);
	}

	// polynomial
	r_set(a, c[COEF_COUNT-1]);
	for (ic=COEF_COUNT-2; ic>=0; ic--)
		r_fma(a, x, c[ic]);

	f(dft, x);

	r_sub(dft, a);
}

void find_t(real t, double p, real start, real end)	// t = (end-start)*p + start
{
	r_rsub(t, end, start);
	r_muld(t, p);
	r_add(t, start);
}

#define slope(ret, p1, p0, pow_p, mult)		slope_full(ret, p1, p0, pow_p, mult, f, start, end, c)
void slope_full(real ret, double p1, double p0, int pow_p, double mult, void (*f)(real,real), real start, real end, real *c)
{
	//	c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));
	//	slope(a, 1.0, 0.5, 2);	r_add(c[n], a);

	static int init=1;
	static real a, dft1, dft0, t1, t0;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(dft1);
		r_init(dft0);
		r_init(t1);
		r_init(t0);
	}

	find_t(t1, p1, start, end);
	find_t(t0, p0, start, end);

	diff_f(dft1, t1, f, c);
	diff_f(dft0, t0, f, c);

	r_sub(dft1, dft0);

	r_sub(t1, t0);
	r_powi(t1, pow_p);

	r_rdiv(ret, dft1, t1);

	r_muld(ret, mult);
}

#define dft_sum(ret, p1, p0, method, mult)	dft_sum_full(ret, p1, p0, method, mult, f, start, end, c)
void dft_sum_full(real ret, double p1, double p0, int method, double mult, void (*f)(real,real), real start, real end, real *c)
{
	// method 1:	c0 += 0.5 * (dft(1.0) + dft(0.0));
	// method 2:	c0 += 0.5 * dft(0.5);
	// call using:	dft_sum(a, 1.0, 0.0, 1);	r_muld(a, 0.5);	r_add(c[n], a);
	// or:		dft_sum(a, 0.5, 0.0, 2);	r_muld(a, 0.5);	r_add(c[n], a);

	static int init=1;
	static real a, dft1, dft0, t1, t0;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(dft1);
		r_init(dft0);
		r_init(t1);
		r_init(t0);
	}

	find_t(t1, p1, start, end);
	diff_f(dft1, t1, f, c);

	if (method==1)
	{
		find_t(t0, p0, start, end);
		diff_f(dft0, t0, f, c);

		r_radd(ret, dft1, dft0);
	}
	else if (method==2)
		r_set(ret, dft1);

	r_muld(ret, mult);
}

void find_polynomial_fit_old2(void (*f)(real,real), real start, real end, real *c, int order)
{
	static int init=1;
	static real a, b;
	int n;
	double	cep1=0.5-0.5*sqrt(0.5),
		cep2=0.5+0.5*sqrt(0.5);		// cubic error peaks
	double	qep1 = (3.-sqrt(5.))/8.,	// quartic error peaks
		qep2 = (5.-sqrt(5.))/8.,
		qep3 = (3.+sqrt(5.))/8.,
		qep4 = (5.+sqrt(5.))/8.;
	double	sep1 = (2.-sqrt(3.))/4.,	// sextic error peaks
		sep2 = (2.+sqrt(3.))/4.;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(b);
	}

	if (order >= 0)
	{
		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		r_add(c[n], a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
	}

	if (order >= 1)
	{
		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		r_add(c[n], a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
		n=0;	dft_sum(a, 0.5, 0.0, 2, 0.5);		r_add(c[n], a);		//c0 += 0.5 * dft(0.5);
	}

	if (order >= 2)
	{
		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		r_add(c[n], a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=1;	slope(a, 0.75, 0.0, 1, 0.5);					//c1 += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
			slope(b, 1.0, 0.25, 1, 0.5);		r_add(c[n], a);		//    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));
								r_add(c[n], b);

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		r_add(c[n], a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
	}

	if (order >= 3)
	{
		n=3;	slope(a, 0.75, 0.5, 3, -0.25);					//c3 += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
			slope(b, 0.25, 0.5, 3, -0.25);		r_add(c[n], a);		//     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));
								r_add(c[n], b);

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		r_add(c[n], a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		r_add(c[n], a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
		n=0;	dft_sum(a, cep1, cep2, 2, 0.5);		r_add(c[n], a);		//c0 += 0.5 * (dft(cep1) + dft(cep2));
	}

	if (order >= 4)
	{
		n=4;	slope(a, cep1, 0.5, 4, -0.5);					//c4 += -0.5 * (dft(cep1) - dft(0.5)) / sq(sq(t(cep1) - t(0.5)))
			slope(b, cep2, 0.5, 4, -0.5);		r_add(c[n], a);		//     - 0.5 * (dft(cep2) - dft(0.5)) / sq(sq(t(cep2) - t(0.5)));
								r_add(c[n], b);

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		r_add(c[n], a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=1;	slope(a, 0.75, 0.0, 1, 0.5);					//c1 += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
			slope(b, 1.0, 0.25, 1, 0.5);		r_add(c[n], a);		//    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));
								r_add(c[n], b);

		n=3;	slope(a, 0.75, 0.5, 3, -0.25);					//c3 += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
			slope(b, 0.25, 0.5, 3, -0.25);		r_add(c[n], a);		//     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));
								r_add(c[n], b);

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		r_add(c[n], a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=1;	slope(a, qep3, qep1, 1, 0.5);					//c1 += 0.5 * (dft(qep3) - dft(qep1)) / (t(qep3) - t(qep1))
			slope(b, qep4, qep2, 1, 0.5);		r_add(c[n], a);		//    + 0.5 * (dft(qep4) - dft(qep2)) / (t(qep4) - t(qep2));
								r_add(c[n], b);

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		r_add(c[n], a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
	}

	if (order >= 5)
	{
		n=5;	slope(a, qep3, qep2, 5, 0.72131471451);	r_add(c[n], a);		//c5 += 0.72131471451 * (dft(qep3) - dft(qep2)) / pow(t(qep3) - t(qep2), 5.);	// why 0.72131471451?

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		r_add(c[n], a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=1;	slope(a, 0.75, 0.0, 1, 0.5);					//c1 += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
			slope(b, 1.0, 0.25, 1, 0.5);		r_add(c[n], a);		//    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));
								r_add(c[n], b);

		n=3;	slope(a, 0.75, 0.5, 3, -0.25);					//c3 += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
			slope(b, 0.25, 0.5, 3, -0.25);		r_add(c[n], a);		//     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));
								r_add(c[n], b);

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		r_add(c[n], a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		// error curve is now mostly quartic
		n=4;	slope(a, cep1, 0.5, 4, -0.5);					//c4 += -0.5 * (dft(cep1) - dft(0.5)) / sq(sq(t(cep1) - t(0.5)))
			slope(b, cep2, 0.5, 4, -0.5);		r_add(c[n], a);		//     - 0.5 * (dft(cep2) - dft(0.5)) / sq(sq(t(cep2) - t(0.5)));
								r_add(c[n], b);

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		r_add(c[n], a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=1;	slope(a, 0.75, 0.0, 1, 0.5);					//c1 += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
			slope(b, 1.0, 0.25, 1, 0.5);		r_add(c[n], a);		//    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));
								r_add(c[n], b);

		n=3;	slope(a, 0.75, 0.5, 3, -0.25);					//c3 += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
			slope(b, 0.25, 0.5, 3, -0.25);		r_add(c[n], a);		//     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));
								r_add(c[n], b);

		n=1;	slope(a, qep3, qep1, 1, 0.5);					//c1 += 0.5 * (dft(qep3) - dft(qep1)) / (t(qep3) - t(qep1))
			slope(b, qep4, qep2, 1, 0.5);		r_add(c[n], a);		//    + 0.5 * (dft(qep4) - dft(qep2)) / (t(qep4) - t(qep2));
								r_add(c[n], b);

		n=2;	slope(a, sep1, 0.5, 2, 0.5);					//c2 += 0.5 * (dft(sep1) - dft(0.5)) / sq(t(sep1) - t(0.5))
			slope(b, sep2, 0.5, 2, 0.5);		r_add(c[n], a);		//    + 0.5 * (dft(sep2) - dft(0.5)) / sq(t(sep2) - t(0.5));
								r_add(c[n], b);

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		r_add(c[n], a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		r_add(c[n], a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
		n=0;	dft_sum(a, 0.5, 0.0, 2, 0.5);		r_add(c[n], a);		//c0 += 0.5 * dft(0.5);
	}

	if (order >= 6)
	{
		n=6;	slope(a, 1.0, 0.5, 6, 1.);		r_add(c[n], a);
	}
}

void add_to_coef(real cn, int mult, real coef, real pos, int power)	// does c[n] += mult*coef*pos^power
{
	static int init=1;
	static real a;

	if (init)
	{
		init = 0;
		r_init(a);
	}

	r_rpowi(a, pos, power);
	r_mul(a, coef);
	r_muli(a, mult);
	r_add(cn, a);
}

#define atc(a)	add_translated_coef(a, mid, n, c)
void add_translated_coef(real coef, real pos_o, int n, real *c)	// adds coef*(x-pos_o)^n to c
{
	static int init=1;
	static real pos;

	if (init)
	{
		init = 0;
		r_init(pos);
	}

	r_rneg(pos, pos_o);

	switch (n)
	{
		case 0:
			r_add(c[0], coef);			// c0 += coef
			break;

		case 1:
			add_to_coef(c[0], 1, coef, pos, 1);	// c0 += coef*pos
			r_add(c[1], coef);			// c1 += coef
			break;

		case 2:
			add_to_coef(c[0], 1, coef, pos, 2);	// c0 += coef*pos^2
			add_to_coef(c[1], 2, coef, pos, 1);	// c1 += 2*coef*pos
			r_add(c[2], coef);			// c2 += coef
			break;

		case 3:
			add_to_coef(c[0], 1, coef, pos, 3);	// c0 += coef*pos^3
			add_to_coef(c[1], 3, coef, pos, 2);	// c1 += 3*coef*pos^2
			add_to_coef(c[2], 3, coef, pos, 1);	// c2 += 3*coef*pos
			r_add(c[3], coef);			// c3 += coef
			break;

		case 4:
			add_to_coef(c[0], 1, coef, pos, 4);	// c0 += coef*pos^4
			add_to_coef(c[1], 4, coef, pos, 3);	// c1 += 4*coef*pos^3
			add_to_coef(c[2], 6, coef, pos, 2);	// c2 += 6*coef*pos^2
			add_to_coef(c[3], 4, coef, pos, 1);	// c3 += 4*coef*pos
			r_add(c[4], coef);			// c4 += coef
			break;

		case 5:
			add_to_coef(c[0], 1, coef, pos, 5);	// c0 += coef*pos^5
			add_to_coef(c[1], 5, coef, pos, 4);	// c1 += 5*coef*pos^4
			add_to_coef(c[2], 10, coef, pos, 3);	// c2 += 10*coef*pos^3
			add_to_coef(c[3], 10, coef, pos, 2);	// c3 += 10*coef*pos^2
			add_to_coef(c[4], 5, coef, pos, 1);	// c4 += 5*coef*pos
			r_add(c[5], coef);			// c5 += coef
			break;

		case 6:
			add_to_coef(c[0], 1, coef, pos, 6);	// c0 += coef*pos^6
			add_to_coef(c[1], 6, coef, pos, 5);	// c1 += 6*coef*pos^5
			add_to_coef(c[2], 15, coef, pos, 2);	// c2 += 15*coef*pos^2
			add_to_coef(c[3], 20, coef, pos, 3);	// c3 += 20*coef*pos^3
			add_to_coef(c[4], 15, coef, pos, 2);	// c4 += 15*coef*pos^2
			add_to_coef(c[5], 6, coef, pos, 1);	// c5 += 6*coef*pos
			r_add(c[6], coef);			// c6 += coef
			break;
	}
}

void find_polynomial_fit_old(void (*f)(real,real), real start, real end, real *c, int order, double k)
{
	static int init=1;
	static real a, b, mid;
	int n;
	double	cep1=0.5-0.5*sqrt(0.5),
		cep2=0.5+0.5*sqrt(0.5);		// cubic error peaks
	double	qep1 = (3.-sqrt(5.))/8.,	// quartic error peaks
		qep2 = (5.-sqrt(5.))/8.,
		qep3 = (3.+sqrt(5.))/8.,
		qep4 = (5.+sqrt(5.))/8.;
	double	sep1 = (2.-sqrt(3.))/4.,	// sextic error peaks
		sep2 = (2.+sqrt(3.))/4.;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(b);
		r_init(mid);
	}

	r_radd(mid, start, end);
	r_muld(mid, 0.5);

	if (order >= 0)
	{
		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		atc(a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
	}

	if (order >= 1)
	{
		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		atc(a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		if (order==1)
		{
			n=0;	dft_sum(a, 0.5, 0.0, 2, 0.5);	atc(a);		//c0 += 0.5 * dft(0.5);
		}
	}

	if (order >= 2)
	{
		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		atc(a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		atc(a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));

		n=1;	slope(a, 0.75, 0.0, 1, 0.5);				//c1 += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
			slope(b, 1.0, 0.25, 1, 0.5);	atc(a);	atc(b);		//    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));
			
	}

	if (order >= 3)
	{
		n=3;	slope(a, 0.75, 0.5, 3, -0.25);				//c3 += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
			slope(b, 0.25, 0.5, 3, -0.25);	atc(a);	atc(b);		//     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		atc(a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=0;	dft_sum(a, cep1, cep2, 2, 0.5);		atc(a);		//c0 += 0.5 * (dft(cep1) + dft(cep2));
	}

	if (order >= 4)
	{
		n=4;	slope(a, cep1, 0.5, 4, -0.5);				//c4 += -0.5 * (dft(cep1) - dft(0.5)) / sq(sq(t(cep1) - t(0.5)))
			slope(b, cep2, 0.5, 4, -0.5);	atc(a);	atc(b);		//     - 0.5 * (dft(cep2) - dft(0.5)) / sq(sq(t(cep2) - t(0.5)));

		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		atc(a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=1;	slope(a, qep3, qep1, 1, 0.5);				//c1 += 0.5 * (dft(qep3) - dft(qep1)) / (t(qep3) - t(qep1))
			slope(b, qep4, qep2, 1, 0.5);	atc(a);	atc(b);		//    + 0.5 * (dft(qep4) - dft(qep2)) / (t(qep4) - t(qep2));

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		atc(a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
	}

	if (order >= 5)
	{
		n=5;	slope(a, qep3, qep2, 5, 0.72132221);	atc(a);		//c5 += 0.72131471451 * (dft(qep3) - dft(qep2)) / pow(t(qep3) - t(qep2), 5.);	// why 0.72131471451?

		n=1;	slope(a, 0.75, 0.0, 1, 0.5);				//c1 += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
			slope(b, 1.0, 0.25, 1, 0.5);	atc(a);	atc(b);		//    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));

		n=3;	slope(a, 0.75, 0.5, 3, -0.25);				//c3 += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
			slope(b, 0.25, 0.5, 3, -0.25);	atc(a);	atc(b);		//     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		atc(a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=2;	slope(a, sep1, 0.5, 2, 0.5);				//c2 += 0.5 * (dft(sep1) - dft(0.5)) / sq(t(sep1) - t(0.5))
			slope(b, sep2, 0.5, 2, 0.5);	atc(a);	atc(b);		//    + 0.5 * (dft(sep2) - dft(0.5)) / sq(t(sep2) - t(0.5));

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		atc(a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
		n=0;	dft_sum(a, 0.5, 0.0, 2, 0.5);		atc(a);		//c0 += 0.5 * dft(0.5);
	}

	if (order >= 6)
	{
		n=6;	slope(a, sep1, 1.0, 6, -337.78998222343);
			slope(b, sep2, 0.0, 6, -337.78998222343);	atc(a);	atc(b);
		//n=6;	slope(a, sep1, 1.0, 6, k);
		//	slope(b, sep2, 0.0, 6, k);	atc(a);	atc(b);

		n=1;	slope(a, 1.0, 0.0, 1, 1.0);		atc(a);		//c1 += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		atc(a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		r_add(c[n], a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));

		n=4;	slope(a, cep1, 0.5, 4, -0.5);				//c4 += -0.5 * (dft(cep1) - dft(0.5)) / sq(sq(t(cep1) - t(0.5)))
			slope(b, cep2, 0.5, 4, -0.5);	atc(a);	atc(b);		//     - 0.5 * (dft(cep2) - dft(0.5)) / sq(sq(t(cep2) - t(0.5)));

		n=2;	slope(a, 1.0, 0.5, 2, 1.0);		r_add(c[n], a);		//c2 += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		n=1;	slope(a, 1.0-0.01455, 0.01455, 1, 1.0);	atc(a);

		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		atc(a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
	}

}

//#include "../rouziclib/math/matrix.c"
void fit_polynomial_on_points(double *x, double *y, double *c, int order)
{
	int i, j, n=order+1;
	static double a[COEF_COUNT*COEF_COUNT], ainv[COEF_COUNT*COEF_COUNT];

	for (i=0; i<n; i++)
		for (j=0; j<n; j++)
			a[i*n + order-j] = pow(x[i], (double) j);	// makes each row be something like [ xi^order ... xi^3   xi^2   xi   1 ]

	matrix_inverse(a, ainv, n);
	matrix_multiplication(ainv, y, c, n, n, 1);

	printf("Matrix A:\n");
	for (i=0; i<n; i++)
			printf("%8g%s", y[i], i==n-1 ? "\n" : "\t");
	//	for (j=0; j<n; j++)
	//		printf("%8g%s", ainv[i*n+j], j==n-1 ? "\n" : "\t");
}

void find_polynomial_fit(void (*f)(real,real), real start, real end, real *c, int order, double k)
{
	static int init=1;
	static real a, b, t;
	int i, n;
	double dstart=r_todouble(start), dend=r_todouble(end);;
	static double *zcd[7], zc[7*(7+1)/2], *ppd[7], pp[7*(7+1)/2];
	static double array_x[COEF_COUNT], array_y[COEF_COUNT], array_c[COEF_COUNT];

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(b);
		r_init(t);

		zcd[0] = &zc[0];
		for (i=1; i<sizeof(zcd)/sizeof (*zcd); i++)
			zcd[i] = zcd[i-1] + i;		// point to the right zero-crossings for each degree

		ppd[0] = &pp[0];
		for (i=1; i<sizeof(ppd)/sizeof (*ppd); i++)
			ppd[i] = ppd[i-1] + i;		// point to the right peaks for each degree

		// Zero-crossings by degree
		zcd[0][0] = 0.;

		zcd[1][0] = -sqrt(0.5);
		zcd[1][1] = sqrt(0.5);

		zcd[2][0] = -sqrt(3.)/2.;
		zcd[2][1] = 0.;
		zcd[2][2] = sqrt(3.)/2.;

		zcd[3][0] = -sqrt(2.+sqrt(2.))/2.;
		zcd[3][1] = -sqrt(2.-sqrt(2.))/2.;
		zcd[3][2] = sqrt(2.-sqrt(2.))/2.;
		zcd[3][3] = sqrt(2.+sqrt(2.))/2.;

		zcd[4][0] = -sqrt(2.5+sqrt(5.)/2.)/2.;
		zcd[4][1] = -sqrt(2.5-sqrt(5.)/2.)/2.;
		zcd[4][2] = 0.;
		zcd[4][3] = sqrt(2.5-sqrt(5.)/2.)/2.;
		zcd[4][4] = sqrt(2.5+sqrt(5.)/2.)/2.;

		zcd[5][0] = -sqrt(2.+sqrt(3.))/2.;
		zcd[5][1] = -sqrt(0.5);
		zcd[5][2] = -sqrt(2.-sqrt(3.))/2.;
		zcd[5][3] = sqrt(2.-sqrt(3.))/2.;
		zcd[5][4] = sqrt(0.5);
		zcd[5][5] = sqrt(2.+sqrt(3.))/2.;

		zcd[6][0] = -0.9749279121818236;
		zcd[6][1] = -0.781831;
		zcd[6][2] = -0.433884;
		zcd[6][3] = 0.;
		zcd[6][4] = 0.433884;
		zcd[6][5] = 0.781831;
		zcd[6][6] = 0.9749279121818236;

		// Polynomial peaks by degree
		ppd[0][0] = 0.;

		ppd[1][0] = -1.;
		ppd[1][1] =  1.;

		ppd[2][0] = -1.;
		ppd[2][1] =  0.;
		ppd[2][2] =  1.;

		ppd[3][0] = -1.;
		ppd[3][1] = -0.5;
		ppd[3][2] =  0.5;
		ppd[3][3] =  1.;

		ppd[4][0] = -1.;
		ppd[4][1] = -sqrt(0.5);
		ppd[4][2] =  0.;
		ppd[4][3] =  sqrt(0.5);
		ppd[4][4] =  1.;

		ppd[5][0] = -1.;
		ppd[5][1] = (-sqrt(5.)-1.)/4.;
		ppd[5][2] = (-sqrt(5.)+1.)/4.;
		ppd[5][3] = ( sqrt(5.)-1.)/4.;
		ppd[5][4] = ( sqrt(5.)+1.)/4.;
		ppd[5][5] =  1.;

		ppd[6][0] = -1.;
		ppd[6][1] = -sqrt(3.)/2.;
		ppd[6][2] = -0.5;
		ppd[6][3] =  0.;
		ppd[6][4] =  0.5;
		ppd[6][5] =  sqrt(3.)/2.;
		ppd[6][6] =  1.;

		for (i=0; i<sizeof(zc)/sizeof (*zc); i++)
			zc[i] = 0.5 + 0.5*zc[i];	// move all zero-crossings from a [-1 , 1] range to a [0 , 1] range

		for (i=0; i<sizeof(pp)/sizeof (*pp); i++)
			pp[i] = 0.5 + 0.5*pp[i];	// move all peaks from a [-1 , 1] range to a [0 , 1] range
	}

	/*if (order >= 0)
	{
		n=0;	dft_sum(a, 1.0, 0.0, 1, 0.5);		atc(a);		//c0 += 0.5 * (dft(1.0) + dft(0.0));
	}*/

	if (order >= 1)
	{
		for (n=1; n<=order; n++)
		{
			for (i=0; i<n+1; i++)		// populate x and y arrays
			{
				array_x[i] = zcd[n][i] * (dend-dstart) + dstart;	// set x at the zero-crossings of the future error curve
				//array_x[i] = ppd[n][i] * (dend-dstart) + dstart;	// set x at the current polynomial peaks
				r_setd(t, array_x[i]);
				diff_f(a, t, f, c);
				array_y[i] = r_todouble(a);				// set y as the current error value for x

			}

			fit_polynomial_on_points(array_x, array_y, array_c, n);

			for (i=0; i<n+1; i++)
				r_addd(c[i], array_c[n-i]);
		}
	}
}

enum { NEGMODE, DIVMODE };
double get_polynomial_error(void (*f)(real,real), real start, real end, real *c, int errmode)
{
	int i, ic;
	static int init=1;
	static real a, b;
	static real x, err;
	static FILE *error_f, *func_f;
	char filename[256];
	int order=0;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(b);
		r_init(x);
		r_init(err);

		for (ic=0; ic<COEF_COUNT; ic++)
			if (r_todouble(c[ic]))
				order = ic;

		sprintf(filename, "error_curves/error_curve_d%d.dat", order);
		//sprintf(filename, "error_curves/error_curve.dat");
		error_f = fopen(filename, "wb");
		func_f = fopen("error_curves/function_curve.dat", "wb");
	}

	r_setd(err, 0.);
	for (i=0; i<=1000; i++)
	{
		find_t(x, (double) i / 1000., start, end);

		// polynomial
		r_set(a, c[COEF_COUNT-1]);
		for (ic=COEF_COUNT-2; ic>=0; ic--)
			r_fma(a, x, c[ic]);

		f(b, x);

		if (i%10==0)
			mpfr_fprintf(func_f, "%Rg\t%Rg\n", x, b);

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

		if (i%10==0)
			mpfr_fprintf(error_f, "%Rg\t%Rg\n", x, a);

		r_abs(a);

		if (mpfr_cmp(err, a) < 0)
			r_set(err, a);
	}

	mpfr_fprintf(error_f, "\n");

	return r_todouble(err);
}

void find_polynomial_local_min(void (*f)(real,real), real start, real end, real *c, int order, int errmode)
{
	int32_t i, j, n;
	double err0, err1, hw, maxerr;
	static int init=1;
	static real a, b, step;

	if (init)
	{
		init = 0;
		r_init(a);
		r_init(b);
		r_init(step);
	}

	err0 = get_polynomial_error(f, start, end, c, errmode);
	maxerr = err0;

	hw = 0.5 * (r_todouble(end) - r_todouble(start));

	for (j=0; j<10; j++)
	{
		for (n=order; n>0; n--)
		{
			r_setd(step, pow(hw, (double) -n) * err0 * 0.001 / pow(2., (double) j));

			r_set(a, c[n]);
			r_set(b, c[n]);

			for (i=0; i<2; i++)
			{
				r_add(c[n], step);
				err1 = get_polynomial_error(f, start, end, c, errmode);

				if (err1 < maxerr)
				{
					r_set(b, c[n]);
					maxerr = err1;
	printf("Error went from %g to %g, %.3f times better (n=%d, i=%d, j=%d)\n", err0, maxerr, err0/maxerr, n, i, j);
				}
			}

			r_set(c[n], a);
			for (i=0; i<2; i++)
			{
				r_sub(c[n], step);
				err1 = get_polynomial_error(f, start, end, c, errmode);

				if (err1 < maxerr)
				{
					r_set(b, c[n]);
					maxerr = err1;
	printf("Error went from %g to %g, %.3f times better (n=%d, i=%d, j=%d)\n", err0, maxerr, err0/maxerr, n, -i, j);
				}
			}

			r_set(c[n], b);
		}
	}

	printf("Error went from %g to %g, %.3f times better\n", err0, maxerr, err0/maxerr);
}

double get_double_impl_error(void (*f)(real,real), double (*g)(double), real start, real end, int errmode)
{
	int32_t i;
	static int init=1;
	static real a, b;
	static real x, err;

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
		find_t(x, (double) i / 1000., start, end);

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

double get_float_impl_error(void (*f)(real,real), float (*g)(float), real start, real end, int errmode)
{
	int32_t i;
	static int init=1;
	static real a, b;
	static real x, err;

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
		find_t(x, (double) i / 1000., start, end);

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

void f_cos(real y, real x)
{
	static int init=1;
	static real a;

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

double find_max_intermediary_value(real *c_r, real segstart_r, real segend_r)
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

//#include "../rouziclib/fixedpoint/ffo_lut.h"

//#define K_TEST
void make_cos_table(const int cos_order, const int lutsp)	// cos_order is <= 6, lutsp can only be <= 15
{
	int32_t i, is, segcount, prec;
	const int luts = pow(2., (double) lutsp) + 0.5;
	real c[COEF_COUNT];
	real start, segstep, segstart, segend;
	double end=0.25, step=end/(double) luts, err, errd, errf, maxerr=0., maxerrd=0., maxerrf=0., v, maxv=0.;
double k, errmin=1., mink;
	FILE *file;
	char header_path[64];

	sprintf(header_path, "../rouziclib/fastfloat/fastcos_d%d.h", cos_order);
	file = fopen(header_path, "wb");
	fprintf(file, "{");

	segcount = ceil(end / step) + 1;	// +1 for the only lookup for x==0.25

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

		if (is!=segcount-1)	// let the final segment be 0
		{
#ifdef K_TEST
errmin=1.;
//for (k=-337.789982223; k>=-337.789982224; k-=1e-11)
//for (k=0.7213219; k<=0.72132221; k+=1e-9)
//for (k=0.014; k<=0.0147; k+=1e-5)
for (k=500.; k>=-550.; k-=1e-0)
{
for (i=0; i<COEF_COUNT; i++)
r_setd(c[i], 0.);
#endif
			//find_polynomial_fit(f_cos, segstart, segend, c, cos_order, k);	// this doesn't really work, very imprecise
			find_polynomial_fit_old(f_cos, segstart, segend, c, cos_order, k);
			//find_polynomial_local_min(f_cos, segstart, segend, c, cos_order, NEGMODE);
			err = get_polynomial_error(f_cos, segstart, segend, c, NEGMODE);
			if (err > maxerr)
				maxerr = err;

#ifdef K_TEST
if (err < errmin)
{
	errmin = err;
	mink = k;
	//printf("k=%.10f, err %g\n", k, err);
}
}
printf("\n-----k=%.12f, err %g\n\n", mink, errmin);
#endif
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
		for (i=cos_order; i>=1; i--)
			if (i==1)
				mpfr_printf("%+.16Rf*x", c[i]);
			else
				mpfr_printf("%+.16Rf*x^%d", c[i], i);
		mpfr_printf("%+.16Rf\n[%.7Rf , %.7Rf]\terr %g\n", c[0], segstart, segend, err);
		//mpfr_printf("%+.16Rf  err %g\n\tdouble err:%g\ti=%3d (segratio 1/%.0f) [%.7Rf , %.7Rf]\n", c[0], err, errd, is, 1./step, segstart, segend);

		for (i=0; i<cos_order; i++)
			mpfr_fprintf(file, "%.20Rg, ", c[i]);
		mpfr_fprintf(file, "%.20Rg%s", c[cos_order], (is==segcount-1) ? "};\n" : ", \n");
	}

	fprintf(file, "const uint32_t lutsp = %d;", lutsp);

	fclose (file);

	if (maxv <= 2047.)
		prec = 63 - ffo_lut[(int) ceil(maxv)];
	else
		prec = -1;
	printf("\nMax error: %g\tMax double implementation error: %g, float implementation error: %g\nMax intermediary value: %g, prec: %d fractional bits\n", maxerr, maxerrd, maxerrf, maxv, prec);
}

// Sinc windowed with squared gaussian window

void r_squared_gaussian_window(real y, real x, real w)
{
	static int init=1;
	static real a, b;

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

real ws_range, ws_sigma_range;
int no_sine=0;

void f_wsinc(real y, real x)
{
	static int init=1;
	static real a;

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
	real c[COEF_COUNT];
	real segstep, segstart, segend;
	double end=wsinc_range, step=1./pow(2., lutsp), err, maxerr=0.;
	FILE *file;
	char header_path[64];

	sprintf(header_path, "../rouziclib/fastfloat/fastwsinc.h", order);
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
		find_polynomial_fit_old(f_wsinc, segstart, segend, c, order, 0.);
		err = get_polynomial_error(f_wsinc, segstart, segend, c, no_sine==0 ? NEGMODE : DIVMODE);
		if (err > maxerr)
			maxerr = err;

		// Printing
		mpfr_printf("[%.7Rf , %.7Rf]\terr %e\n", segstart, segend, err);

		for (i=0; i<order; i++)
			mpfr_fprintf(file, "%.20Rg, ", c[i]);
		mpfr_fprintf(file, "%.20Rg%s", c[order], (is==segcount-1) ? "};\n" : ", \n");
	}

	double ind_off = exp2(double_get_exponent(wsinc_range));	// offset to calculate the LUT index from the offset mantissa
	if (double_get_mantissa(wsinc_range))
		ind_off *= 2.;

	int ish = 52 - (double_get_exponent(ind_off) + lutsp);

	fprintf(file, "const uint32_t order = %d, lutsp = %d, ish = %d;\n", order, lutsp, ish);
	fprintf(file, "const double wsinc_range = %g, ind_off = %g;", wsinc_range, ind_off);

	fclose (file);

	printf("\nMax error: %g\n", maxerr);
	printf("LUT size (double): %d bytes\n", segcount * (order+1) * sizeof(double));
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

int main()
{
	mpfr_set_default_prec(PREC);

	/*make_cos_table(5, 5);
	make_cos_table(2, 6);
*/	make_wsinc_table(2, 5, 6.625, 1.6083);

	//test_matrix();

	//printf("sizeof(long double) = %d (%d bits). LDBL_DIG  = %d, LDBL_MANT_DIG = %d\n", sizeof(long double), sizeof(long double)*8, LDBL_DIG, LDBL_MANT_DIG);
}
