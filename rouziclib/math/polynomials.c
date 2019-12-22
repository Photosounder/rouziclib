double eval_polynomial(double x, double *c, int degree)
{
	int i;
	double y = 0.;

	for (i=degree; i > 0; i--)
		y = (y + c[i]) * x;
	y += c[0];

	return y;
}

#ifdef RL_MPFR
void eval_polynomial_mpfr(real y, real x, real *c, int degree)
{
	int i;

	r_setd(y, 0.);

	for (i=degree; i > 0; i--)
	{
		r_add(y, c[i]);
		r_mul(y, x);
	}
	r_add(y, c[0]);
}
#endif

void polynomial_addition(double *a, int adeg, double *b, int bdeg, double *c)
{
	int i, mindeg = MINN(adeg, bdeg), maxdeg = MAXN(adeg, bdeg);

	for (i=0; i <= maxdeg; i++)
	{
		if (i <= mindeg)
			c[i] = a[i] + b[i];
		else
			c[i] = (adeg > bdeg) ? a[i] : b[i];
	}
}

void polynomial_subtraction(double *a, int adeg, double *b, int bdeg, double *c)
{
	int i, mindeg = MINN(adeg, bdeg), maxdeg = MAXN(adeg, bdeg);

	for (i=0; i <= maxdeg; i++)
	{
		if (i <= mindeg)
			c[i] = a[i] - b[i];
		else
			c[i] = (adeg > bdeg) ? a[i] : -b[i];
	}
}

void polynomial_scalar_mul(double *a, int adeg, double m, double *c)
{
	int i;

	for (i=0; i <= adeg; i++)
		c[i] = a[i] * m;
}

int polynomial_multiplication(double *a, int adeg, double *b, int bdeg, double *c, int cdeg)
{
	int ia, ib, deg, maxdeg=0;

	for (ia=0; ia <= adeg; ia++)
	{
		for (ib=0; ib <= bdeg; ib++)
		{
			deg = ia + ib;

			if (cdeg >= deg)
				c[deg] += a[ia] * b[ib];

			if (c[deg] != 0.)
				maxdeg = MAXN(maxdeg, deg);
		}
	}

	if (cdeg < maxdeg)
		fprintf_rl(stderr, "cdeg of %d < maxdeg of %d in polynomial_multiplication()\n", cdeg, maxdeg);

	return maxdeg;
}

double *polynomial_power(double *a, int adeg, int n, int *maxdegp)
{
	int i, maxdeg=0;
	double *c0, *c1, *p;

	if (n==0)
	{
		c0 = calloc(1, sizeof(double));
		c0[0] = 1.;

		if (maxdegp)
			*maxdegp = 0;
		return c0;
	}

	c1 = calloc(adeg*n+1, sizeof(double));
	c0 = calloc(adeg*n+1, sizeof(double));
	c0[0] = 1.;

	for (i=1; i <= n; i++)
	{
		memset(c1, 0, (adeg*n+1)*sizeof(double));
		maxdeg = polynomial_multiplication(a, adeg, c0, maxdeg, c1, adeg*n);

		p = c0;
		c0 = c1;
		c1 = p;
	}

	free(c1);

	if (maxdegp)
		*maxdegp = maxdeg;
	return c0;
}

void polynomial_x_substitution(double *a, int adeg, double *xs, int xsdeg, double *c)
{
	int i, id, maxdeg;
	double *exs;

	for (id=0; id <= adeg; id++)
	{
		exs = polynomial_power(xs, xsdeg, id, &maxdeg);		// expand xs^id
		for (i=0; i <= maxdeg; i++)
			c[i] += a[id] * exs[i];				// multiply by the coefficient and add to c
		free(exs);
	}
}

#ifdef RL_MPFR
void polynomial_addition_mpfr(real *a, int adeg, real *b, int bdeg, real *c)
{
	int i, mindeg = MINN(adeg, bdeg), maxdeg = MAXN(adeg, bdeg);

	for (i=0; i <= maxdeg; i++)
	{
		if (i <= mindeg)
			r_radd(c[i], a[i], b[i]);
		else
			r_set(c[i], (adeg > bdeg) ? a[i] : b[i]);
	}
}

void polynomial_subtraction_mpfr(real *a, int adeg, real *b, int bdeg, real *c)
{
	int i, mindeg = MINN(adeg, bdeg), maxdeg = MAXN(adeg, bdeg);

	for (i=0; i <= maxdeg; i++)
	{
		if (i <= mindeg)
			r_rsub(c[i], a[i], b[i]);
		else if (adeg > bdeg)
			r_set(c[i], a[i]);
		else
			r_rneg(c[i], b[i]);
	}
}

void polynomial_scalar_mul_mpfr(real *a, int adeg, real m, real *c)
{
	int i;

	for (i=0; i <= adeg; i++)
		r_rmul(c[i], a[i], m);
}

int polynomial_multiplication_mpfr(real *a, int adeg, real *b, int bdeg, real *c, int cdeg)
{
	int ia, ib, deg, maxdeg=0;

	for (ia=0; ia <= adeg; ia++)
	{
		for (ib=0; ib <= bdeg; ib++)
		{
			deg = ia + ib;

			if (cdeg >= deg)
				r_fma(c[deg], a[ia], b[ib], c[deg]);

			if (mpfr_zero_p(c[deg])==0)
				maxdeg = MAXN(maxdeg, deg);
		}
	}

	if (cdeg < maxdeg)
		fprintf_rl(stderr, "cdeg of %d < maxdeg of %d in polynomial_multiplication_mpfr()\n", cdeg, maxdeg);

	return maxdeg;
}

real *polynomial_power_mpfr(real *a, int adeg, int n, int *maxdegp)
{
	int i, j, maxdeg=0;
	real *c0, *c1, *p;

	if (n==0)
	{
		c0 = r_init_array(1);
		r_setd(c0[0], 1.);

		if (maxdegp)
			*maxdegp = 0;
		return c0;
	}

	c0 = r_init_array(adeg*n+1);
	c1 = r_init_array(adeg*n+1);
	r_setd(c0[0], 1.);

	for (i=1; i <= n; i++)
	{
		for (j=0; j < adeg*n+1; j++)
			r_setd(c1[j], 0.);

		maxdeg = polynomial_multiplication_mpfr(a, adeg, c0, maxdeg, c1, adeg*n);

		p = c0;
		c0 = c1;
		c1 = p;
	}

	r_free_array(&c1, adeg*n+1);
	for (i=maxdegp+1; i < adeg*n+1; i++)
		r_free(c0[i]);		// c0 will only have reals up to maxdeg

	if (maxdegp)
		*maxdegp = maxdeg;
	return c0;
}

void polynomial_x_substitution_mpfr(real *a, int adeg, real *xs, int xsdeg, real *c)
{
	int i, id, maxdeg;
	real *exs;

	for (id=0; id <= adeg; id++)
	{
		exs = polynomial_power_mpfr(xs, xsdeg, id, &maxdeg);	// expand xs^id
		for (i=0; i <= maxdeg; i++)
			r_fma(c[i], a[id], exs[i], c[i]);		// multiply by the coefficient and add to c
		r_free_array(&exs, maxdeg+1);
	}
}
#endif

double *chebyshev_coefs(int degree)
{
	int i;
	double *t0, *t1, *t2, *p;
	double m[2] = { 0., 2. };	// 2*x + 0

	t0 = calloc(degree+1, sizeof(double));
	t0[0] = 1.;
	if (degree==0)
		return t0;

	t1 = calloc(degree+1, sizeof(double));
	t1[1] = 1.;
	if (degree==1)
	{
		free(t0);
		return t1;
	}

	t2 = calloc(degree+1, sizeof(double));

	for (i=2; i <= degree; i++)
	{
		// t2 = 2*x * t1 - t0
		memset(t2, 0, (degree+1) * sizeof(double));
		polynomial_multiplication(m, 1, t1, i-1, t2, degree);
		polynomial_subtraction(t2, degree, t0, i-2, t2);

		p = t0;
		t0 = t1;
		t1 = t2;
		t2 = p;
	}

	free(t0);
	free(t2);
	return t1;
}

#ifdef RL_MPFR
real *chebyshev_coefs_mpfr(int degree)
{
	int i;
	real *t0, *t1, *t2, *p, *m;

	t0 = r_init_array(degree+1);
	r_setd(t0[0], 1.);
	if (degree==0)
		return t0;

	t1 = r_init_array(degree+1);
	r_setd(t1[1], 1.);
	if (degree==1)
	{
		r_free_array(&t0, degree+1);
		return t1;
	}

	t2 = r_init_array(degree+1);
	m = r_init_array(2);
	r_setd(m[1], 2.);	// 2*x + 0

	for (i=2; i <= degree; i++)
	{
		// t2 = 2*x * t1 - t0
		r_zero_array(t2, degree+1);
		polynomial_multiplication_mpfr(m, 1, t1, i-1, t2, degree);
		polynomial_subtraction_mpfr(t2, degree, t0, i-2, t2);

		p = t0;
		t0 = t1;
		t1 = t2;
		t2 = p;
	}

	r_free_array(&m, 2);
	r_free_array(&t0, degree+1);
	r_free_array(&t2, degree+1);

	return t1;
}
#endif

double chebyshev_node(int degree, int node)		// node = 0,...,degree-1
{
	if (degree==0 || node >= degree)
		return 0.;

	return cos(pi * ((double) node + 0.5) / (double) degree);
}

#ifdef RL_MPFR
void chebyshev_node_mpfr(real v, int degree, int node)
{
	if (degree==0 || node >= degree)
	{
		r_setd(v, 0.);
		return;
	}

	// cos(pi * ((double) node + 0.5) / (double) degree);
	r_pi(v);
	r_muld(v, (double) node + 0.5);
	r_divd(v, (double) degree);
	r_cos(v, v);
}
#endif

void polynomial_fit_on_points(xy_t *p, double *c, int degree)
{
	int i, ii, id, j, deg;
	double *lj, mj, ci;

	memset(c, 0, (degree+1)*sizeof(double));
	lj = calloc(degree*2, sizeof(double));

	for (j=0; j < degree+1; j++)
	{
		// Prepare the (a*x + b) form Lagrange polynomial coeficients
		for (i=0; i < degree+1; i++)
		{
			if (i != j)
			{
				ii = i - (i > j);		// one place less if i is over the skipped i==j

				mj = 1. / (p[j].x - p[i].x);
				lj[ii*2+1] = mj;		// the a of (a*x + b)
				lj[ii*2] = -p[i].x * mj;	// the b of (a*x + b)
			}
		}

		// Expand the Lagrange polynomial
		for (i=0; i < (1 << degree); i++)
		{
			deg = 0;
			ci = 1.;

			for (id=0; id < degree; id++)			// go through each element of the multiplication being expanded
			{
				deg += get_bit(i, id);			// deg is the degree of the multiplication
				ci *= lj[id*2 + get_bit(i, id)];	// multiply all the elements together into a coefficient
			}

			c[deg] += ci * p[j].y;				// add the yj-multiplied coefficient to the correct degree
		}
	}

	free(lj);
}

#ifdef RL_MPFR
void polynomial_fit_on_points_mpfr(real *x, real *y, real *c, int degree)
{
	int i, ii, id, j, deg;
	real *lj, mj, ci;

	for (i=0; i<degree+1; i++)
		r_setd(c[i], 0.);

	lj = r_init_array(degree*2);

	r_init(mj);
	r_init(ci);

	for (j=0; j < degree+1; j++)
	{
		// Prepare the (a*x + b) form Lagrange polynomial coeficients
		for (i=0; i < degree+1; i++)
		{
			if (i != j)
			{
				ii = i - (i > j);		// one place less if i is over the skipped i==j

				// mj = 1. / (x[j] - x[i]);
				r_rsub(mj, x[j], x[i]);
				r_rddiv(mj, 1., mj);

				r_set(lj[ii*2+1], mj);		// the a of (a*x + b)
				r_rneg(lj[ii*2], x[i]);		// the b of (a*x + b)
				r_mul(lj[ii*2], mj);
			}
		}

		// Expand the Lagrange polynomial
		for (i=0; i < (1 << degree); i++)
		{
			deg = 0;
			r_setd(ci, 1.);

			for (id=0; id < degree; id++)			// go through each element of the multiplication being expanded
			{
				deg += get_bit(i, id);			// deg is the degree of the multiplication
				r_mul(ci, lj[id*2 + get_bit(i, id)]);	// multiply all the elements together into a coefficient
			}

			r_fma(c[deg], ci, y[j], c[deg]);		// add the yj-multiplied coefficient to the correct degree
		}
	}

	r_free(ci);
	r_free(mj);
	r_free_array(&lj, degree*2);
}
#endif

void polynomial_fit_on_function(double (*f)(double), double start, double end, double *c, int degree)
{
	int i;
	xy_t *p;

	p = calloc(degree+1, sizeof(xy_t));

	// Compute the points
	for (i=0; i < degree+1; i++)
	{
		p[i].x = (end-start) * (0.5+0.5*chebyshev_node(degree+1, i)) + start;
		p[i].y = f(p[i].x);
	}

	// Fit
	polynomial_fit_on_points(p, c, degree);

	free(p);
}

#ifdef RL_MPFR
void polynomial_fit_on_function_mpfr(void (*f)(real,real), real start, real end, real *c, int degree)
{
	int i;
	real *x, *y, range;

	// Init
	r_init(range);
	r_rsub(range, end, start);

	x = r_init_array(degree+1);
	y = r_init_array(degree+1);

	// Compute the points
	for (i=0; i < degree+1; i++)
	{
		chebyshev_node_mpfr(x[i], degree+1, i);
		r_muld(x[i], 0.5);
		r_addd(x[i], 0.5);
		r_fma(x[i], range, x[i], start);

		f(y[i], x[i]);
	}

	// Fit
	polynomial_fit_on_points_mpfr(x, y, c, degree);

	// Freeing
	r_free_array(&x, degree+1);
	r_free_array(&y, degree+1);
	r_free(range);
}
#endif

double chebyshev_multiplier_by_dct(double *y, int p_count, int id)	// look for the Chebyshev multiplier of degree id
{
	int i;
	double x, sum=0., freq;

	freq = pi * (double) id / (double) p_count;

	// DCT
	for (x=0.5, i=0; i < p_count; i++, x+=1.)
		sum += y[i] * cos(x * freq);

	// Sum division
	sum *= 2. / (double) p_count;
	if (id==0)
		sum *= 0.5;	// frequency 0 gets its sum halved

	return sum;
}

void polynomial_fit_on_function_by_dct(double (*f)(double), double start, double end, double *c, int degree)
{
	int i, p_count = 1000;
	double *y, cm, *cc, xs[2];

	memset(c, 0, (degree+1)*sizeof(double));

	y = calloc(p_count, sizeof(double));

	// x substitution for the shifting
	xs[0] = -2.*start/(end-start) - 1.;
	xs[1] = 2. / (end-start);

	// Compute the points
	for (i=0; i < p_count; i++)
		y[i] = f((end-start) * (0.5+0.5*chebyshev_node(p_count, i)) + start);

	// Compute the coefficients
	for (i=0; i <= degree; i++)
	{
		cm = chebyshev_multiplier_by_dct(y, p_count, i);	// get the Chebyshev multiplier for degree i
		cc = chebyshev_coefs(i);				// get the default polynomial coeficients
		polynomial_scalar_mul(cc, i, cm, cc);			// apply the multiplier
		polynomial_x_substitution(cc, i, xs, 1, c);		// shift the polynomial and add to c
		free(cc);

		fprintf_rl(stdout, "coef for T_%d = %g\n", i, cm);
	}

	free(y);
}

#ifdef RL_MPFR
void chebyshev_multiplier_by_dct_mpfr(real v, real *y, int p_count, int id)	// look for the Chebyshev multiplier of degree id
{
	int i;
	real x, sum, freq;

	r_init(x);
	r_init(sum);
	r_init(freq);

	r_pi(freq);
	r_muld(freq, (double) id);
	r_divd(freq, (double) p_count);

	// DCT
	r_setd(x, 0.5);
	for (i=0; i < p_count; i++)
	{
		// sum += y[i] * cos(x * freq);
		r_rmul(v, x, freq);
		r_cos(v, v);
		r_fma(sum, y[i], v, sum);
		
		r_addd(x, 1.);
	}

	// Sum division
	// sum *= 2. / (double) p_count;
	r_muld(sum, 2.);
	r_divd(sum, (double) p_count);
	if (id==0)
		r_muld(sum, 0.5);	// frequency 0 gets its sum halved
	r_set(v, sum);

	r_free(x);
	r_free(sum);
	r_free(freq);
}

void polynomial_fit_on_function_by_dct_mpfr(void (*f)(real,real), real start, real end, real *c, int degree)
{
	int i, p_count = 1000;
	real *y, cm, *cc, *xs;

	r_init(cm);

	xs = r_init_array(2);
	y = r_init_array(p_count);
	r_zero_array(c, degree+1);

	// x substitution for the shifting
	r_rsub(xs[1], end, start);	// xs[0] = -2.*start/(end-start) - 1.;
	r_rmuld(xs[0], start, -2.);
	r_div(xs[0], xs[1]);
	r_subd(xs[0], 1.);

	r_rddiv(xs[1], 2., xs[1]);	// xs[1] = 2. / (end-start);

	// Compute the points
	for (i=0; i < p_count; i++)
	{
		// y[i] = f((end-start) * (0.5+0.5*chebyshev_node(p_count, i)) + start);
		chebyshev_node_mpfr(cm, p_count, i);
		r_muld(cm, 0.5);
		r_addd(cm, 0.5);
		r_rsub(y[i], end, start);
		r_fma(cm, y[i], cm, start);
		f(y[i], cm);
	}

	// Compute the coefficients
	for (i=0; i <= degree; i++)
	{
		chebyshev_multiplier_by_dct_mpfr(cm, y, p_count, i);	// get the Chebyshev multiplier for degree i
		cc = chebyshev_coefs_mpfr(i);				// get the default polynomial coeficients
		polynomial_scalar_mul_mpfr(cc, i, cm, cc);		// apply the multiplier
		polynomial_x_substitution_mpfr(cc, i, xs, 1, c);	// shift the polynomial and add to c
		r_free_array(&cc, i+1);
	}

	r_free_array(&xs, 2);
	r_free_array(&y, degree+1);
	r_free(cm);
}
#endif
