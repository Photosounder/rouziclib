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
void eval_polynomial_mpfr(real_t y, real_t x, real_t *c, int degree)
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

double get_polynomial_error(double (*f)(double), double start, double end, double *c, int degree, int errmode)
{
	int i;
	double x, y, fx, err;

	err = 0.;
	for (i=0; i<=1000; i++)
	{
		x = (double) i / 1000.;
		x = x * (end-start) + start;
		y = eval_polynomial(x, c, degree);
		fx = f(x);

		if (errmode==DIVMODE)
		{
			if (y < fx)
				y = fx / y - 1.;
			else
				y = y / fx - 1.;
		}
		else
			y = fabs(fx - y);
		err = MAXN(err, y);
	}

	return err;
}

double get_polynomial_error_from_points(double *x, double *y, int p_count, double *c, int degree, int errmode)
{
	int i;
	double err, yc;

	for (err=0., i=0; i < p_count; i++)
	{
		yc = eval_polynomial(x[i], c, degree);		

		if (errmode==DIVMODE)
		{
			if (yc < y[i])
				yc = y[i] / yc - 1.;
			else
				yc = yc / y[i] - 1.;
		}
		else
			yc = fabs(y[i] - yc);
		err = MAXN(err, yc);
	}

	return err;
}

char *print_polynomial(double *c, int degree, const char *x)
{
	int id, first=1;
	buffer_t buf={0};

	for (id=degree; id >= 0; id--)
	{
		if (fabs(c[id]) >= 1e-12)
		{
			if (first==0)
				bufprintf(&buf, " %c ", c[id] < 0 ? '-' : '+');

			bufprintf(&buf, "%g", first ? c[id] : fabs(c[id]));

			if (id > 0)
				bufprintf(&buf, "*%s", x);

			if (id > 1)
				bufprintf(&buf, "^%d", id);
			first = 0;
		}
	}

	return buf.buf;
}

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
void polynomial_addition_mpfr(real_t *a, int adeg, real_t *b, int bdeg, real_t *c)
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

void polynomial_subtraction_mpfr(real_t *a, int adeg, real_t *b, int bdeg, real_t *c)
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

void polynomial_scalar_mul_mpfr(real_t *a, int adeg, real_t m, real_t *c)
{
	int i;

	for (i=0; i <= adeg; i++)
		r_rmul(c[i], a[i], m);
}

int polynomial_multiplication_mpfr(real_t *a, int adeg, real_t *b, int bdeg, real_t *c, int cdeg)
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

real_t *polynomial_power_mpfr(real_t *a, int adeg, int n, int *maxdegp)
{
	int i, j, maxdeg=0;
	real_t *c0, *c1, *p;

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

void polynomial_x_substitution_mpfr(real_t *a, int adeg, real_t *xs, int xsdeg, real_t *c)
{
	int i, id, maxdeg;
	real_t *exs;

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
real_t *chebyshev_coefs_mpfr(int degree)
{
	int i;
	real_t *t0, *t1, *t2, *p, *m;

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

double chebyshev_node(double degree, double node)		// node = 0,...,degree-1
{
	if (degree < 1. || node+0.5 < 0. || node+0.5 > degree)
		return 0.;

	return cos(pi * (node + 0.5) / degree);
}

#ifdef RL_MPFR
void chebyshev_node_mpfr(real_t v, double degree, double node)
{
	if (degree < 1. || node+0.5 < 0. || node+0.5 > degree)
	{
		r_setd(v, 0.);
		return;
	}

	// cos(pi * (node + 0.5) / degree);
	r_pi(v);
	r_muld(v, node + 0.5);
	r_divd(v, degree);
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
void polynomial_fit_on_points_mpfr(real_t *x, real_t *y, real_t *c, int degree)
{
	int i, ii, id, j, deg;
	real_t *lj, mj, ci;

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
void polynomial_fit_on_function_mpfr(void (*f)(real_t,real_t), real_t start, real_t end, real_t *c, int degree)
{
	int i;
	real_t *x, *y, range;

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
	double x, sum=0., freq = pi * (double) id / (double) p_count;

	// DCT
	for (x=0.5, i=0; i < p_count; i++, x+=1.)
		sum += y[i] * cos(x * freq);

	// Sum division
	sum *= 2. / (double) p_count;
	if (id==0)
		sum *= 0.5;	// frequency 0 gets its sum halved

	return sum;
}

void polynomial_fit_on_points_by_dct(double *y, int p_count, double start, double end, double *c, int degree)
{
	int id;
	double cm, *cc, xs[2];

	memset(c, 0, (degree+1)*sizeof(double));

	// x substitution for the shifting
	xs[0] = -2.*start/(end-start) - 1.;
	xs[1] = 2. / (end-start);

	// Compute the coefficients
	for (id=0; id <= degree; id++)
	{
		cm = chebyshev_multiplier_by_dct(y, p_count, id);	// get the Chebyshev multiplier for degree id
		cc = chebyshev_coefs(id);				// get the default polynomial coeficients
		polynomial_scalar_mul(cc, id, cm, cc);			// apply the multiplier
		polynomial_x_substitution(cc, id, xs, 1, c);		// shift the polynomial and add to c
		free(cc);
	}
}

void polynomial_fit_on_function_by_dct(double (*f)(double), double start, double end, double *c, int degree)
{
	int i, p_count = 1000;
	double *y = calloc(p_count, sizeof(double));

	// Compute the points
	for (i=0; i < p_count; i++)
		y[i] = f((end-start) * (0.5+0.5*chebyshev_node(p_count, i)) + start);

	// Fit
	polynomial_fit_on_points_by_dct(y, p_count, start, end, c, degree);
	free(y);
}

/*
v0 = 10*x
plot v1 = erf(v0)
plot v2 = -1.8812e-10*v0^7 + 5.95468e-07*v0^5 - 0.000591814*v0^3 + 0.211477*v0
plot v3 = 4.60844e-11*v0^7 - 1.46041e-07*v0^5 + 0.000141684*v0^3 - 0.0401948*v0
plot v4 = (v1 - (v2 + c0*v3))*10


plot v0 = exp(-x*x*100)
v1 = 12.3198*x^8 - 27.945*x^6 + 21.227*x^4 - 6.03238*x^2 + 0.476735
plot v2 = cos(acos(x)*  0)*0.120757+
cos(acos(x)*  2)*-0.123411+cos(acos(x)*  4)*0.0832623+
cos(acos(x)*  6)*-0.0591896+cos(acos(x)*  8)*0.0425347+
cos(acos(x)* 10)*-0.0304192+cos(acos(x)* 12)*0.0214769+
cos(acos(x)* 14)*-0.0148945+cos(acos(x)* 16)*0.0101084+
cos(acos(x)* 18)*-0.0066917+cos(acos(x)* 20)*0.00430733+
cos(acos(x)* 22)*-0.00268643+cos(acos(x)* 24)*0.0016166+
cos(acos(x)* 26)*-0.000933467+cos(acos(x)* 28)*0.000513242+
cos(acos(x)* 30)*-0.000265594+cos(acos(x)* 32)*0.000126873
plot v3 = abs(v0-v1)/v2*0.3066*1
v4 = 6.933738469e-1 +x*-1.537650315e-3 +x^2*-1.384847511e+1
+x^3*8.228637481e-2 +x^4*6.131494002e+1 +x^5*-5.585291021e-1
+x^6*-9.252052240e+1 +x^7*7.844065307e-1 +x^8*4.436068364e+1
plot v5 = abs(v0-v4)*1
plot v6 = 1e-9/(x-c0)

plot v0 = gaussian((0.5+0.5*x)*10)
plot v1 = (3.24426*x^7 - 0.635562*x^6 - 6.03683*x^5 + 1.69759*x^4
 + 2.55079*x^3 - 0.548307*x^2 - 0.260136*x + 0.0219204 - v0)*10
plot v3 = (cos(acos(x)*  0)*0.0338554+
cos(acos(x)*  1)*-0.0188564+cos(acos(x)*  2)*0.00204312+
cos(acos(x)*  3)*0.00394375+cos(acos(x)*  4)*-0.00429958+
cos(acos(x)*  5)*0.00256535+cos(acos(x)*  6)*-0.000839876+
cos(acos(x)*  7)*-0.000108108+cos(acos(x)*  8)*0.000347918+
cos(acos(x)*  9)*-0.000237706)
plot v4 = (1.639233702893998064336560647631011889651e-2
+x*-2.928766252335909864193805457528753979412e-1
+x^2*-4.474539206131717132308014291922265223296e-1
+x^3*2.787075293253191929012413648529849594347
+x^4*1.426790345951435341016863501393847789548
+x^5*-6.479284939695181168932746533466860369859
+x^6*-4.536696035358643058644213203913443690196e-1
+x^7*3.485086271675580226339713430689886173452 - v0) * 10
*/

/*double minmax_find_k(int p_count, double *y, double *a, double *b)	// find k that minimises the difference between y and a+k*b
{
	int i, it;
	double k0=0., k1=0., k2=1.;
	double err0, err1, err2;
	double step=4., decrement=1./2.;

	// Initial error
	for (err0=0., i=0; i < p_count; i++)
		err0 = MAXN(err0, fabs(y[i] - a[i]));
	err2 = err1 = err0;

	for (it=0; it < 200; it++)
	{
		// Eval error
		for (err2=0., i=0; i < p_count; i++)
			err2 = MAXN(err2, fabs(y[i] - (a[i] + k2*b[i])));

		// Check if the precision is high enough
		if (err0 / err1 - 1. < 1e-8 && err1 / err2 - 1. < 1e-8 && err0 != err1)
			return k1;

		if (err2 <= err1)			// if the error is still going down
		{
			err0 = err1;
			err1 = err2;
			k0 = k1;
			k1 = k2;
			k2 *= step;			// continue forward
		}
		else					// if the error is now going back up
		{
			k1 = k0;
			if (k0==0.)
				k2 *= 1./8.;
			else
				k2 = k0;		// go back to k0
			err2 = err1 = err0;
			step = pow(step, decrement);	// lower the step
			k2 *= step;
		}
	}

	return k2;
}*/

double minmax_find_k(int p_count, double *y, double *a, double *b, double *an, double *ye, int degree)
{
	int i, it;
	double k0=0., k1=0., k2=1.;
	double err0, err1, err2;
	double step=4., decrement=1./2.;

	// Initial error
	for (err0=0., i=0; i < p_count; i++)
		err0 = MAXN(err0, fabs(y[i] - a[i]));
	err2 = err1 = err0;

	for (it=0; it < 200; it++)
	{
		// Generate an = a + k*b
		for (i=0; i < p_count; i++)
			an[i] = a[i] + k2 * b[i];

		// Fit an to generate ye
		memset(ye, 0, p_count * sizeof(double));
		for (int id=0; id < degree+1; id++)
		{
			double xi, freq = pi * (double) id / (double) p_count;
			double cm = chebyshev_multiplier_by_dct(an, p_count, id);

			for (xi=0.5, i=0; i < p_count; i++, xi+=1.)
				ye[i] += cos(xi * freq) * cm;
		}

		// Eval error
		for (err2=0., i=0; i < p_count; i++)
			err2 = MAXN(err2, fabs(y[i] - ye[i]));

		// Check if the precision is high enough
		if (err0 / err1 - 1. < 1e-8 && err1 / err2 - 1. < 1e-8 && err0 != err1)
			return k1;

		if (err2 <= err1)			// if the error is still going down
		{
			err0 = err1;
			err1 = err2;
			k0 = k1;
			k1 = k2;
			k2 *= step;			// continue forward
		}
		else					// if the error is now going back up
		{
			k1 = k0;
			if (k0==0.)
				k2 *= 1./8.;
			else
				k2 = k0;		// go back to k0
			err2 = err1 = err0;
			step = pow(step, decrement);	// lower the step
			k2 *= step;
		}
	}

	return k2;
}

/*
plot v1 = graph(3+floor(c0)*5, x)
c0 = 0 0 59
*/

void error_curve_transform(double (*f)(double), int p_count, double start, double end, double *c, int degree, double *ye, double *env, double *env2, double *b, double *x, int it)
{
	int i;
	double mx, v, env_sum, env_mul, off;
	char name[64];

	// Generate the imaginary part of ye for the error envelope
	memset(env, 0, p_count * sizeof(double));
	fprintf_rl(stdout, "ye coefs:\n");
	for (int id=0; id < MINN(p_count, (degree+1)*8); id++)
	{
		double xi, freq = pi * (double) id / (double) p_count;
		double cm = chebyshev_multiplier_by_dct(ye, p_count, id);

		if (fabs(cm) > 1e-4)
			fprintf_rl(stdout, "cos(acos(x)*%3d)*%g+\n", id, cm);
		for (xi=0.5, i=0; i < p_count; i++, xi+=1.)
			env[i] += sin(xi * freq) * cm;
	}

	off = sqrt(sq(ye[0]) + sq(env[0]));

	// Generate reciprocal of the error envelope
	for (i=0; i < p_count; i++)
		env[i] = sqrt(sq(ye[i]) + sq(env[i]));
	math_graph_add(x, env, p_count, sprintf_ret(name, "Envelope #%d", it));

	// Offset the envelope
	for (i=0; i < p_count; i++)
		if (it > 20)
			env2[i] = env[i];
		else
			env2[i] = env[i]*0.1 + off;

	// Cumulate and normalise the envelope to produce a position map
	for (env_sum=0., i=0; i < p_count; i++)
		env_sum += env2[i];
	env_mul = 1. / env_sum;

	for (env_sum=0., i=0; i < p_count; i++)
	{
		v = env2[i];
		env_sum += v;
		mx = (env_sum - 0.5*v) * env_mul;			// mapped x within [0 , 1]
		mx = (end-start) * (0.5+0.5*cos(pi * mx)) + start;	// mapped x within [start , end]

		// Generate the b curve
		b[i] = -(f(mx) - eval_polynomial(mx, c, degree)) / env[i];
	}

	math_graph_add(x, b, p_count, sprintf_ret(name, "B curve #%d", it));
}

void polynomial_fit_on_function_by_dct_minmax(double (*f)(double), double start, double end, double *c, int degree)
{
	int i, it, p_count = 10000;
	char *str;
	const char x_str[] = "x";
	char name[64];
	double v, k, err;
	double *x = calloc(p_count, sizeof(double));
	double *y = calloc(p_count, sizeof(double));
	double *a = calloc(p_count, sizeof(double));
	double *an = calloc(p_count, sizeof(double));
	double *ye = calloc(p_count, sizeof(double));
	double *env = calloc(p_count, sizeof(double));
	double *env2 = calloc(p_count, sizeof(double));
	double *b = calloc(p_count, sizeof(double));
	double *ce = calloc(degree+1, sizeof(double));

	free_math_graph_array();

	// Compute the points
	for (i=0; i < p_count; i++)
	{
		x[i] = (end-start) * (0.5+0.5*chebyshev_node(p_count, i)) + start;
		y[i] = f(x[i]);
	}
	math_graph_add(x, y, p_count, "Original curve to fit");
//	math_graph_add(x, y, p_count, sprintf_ret(name, ""));

	// Fit
	polynomial_fit_on_points_by_dct(y, p_count, start, end, c, degree);
	fprintf_rl(stdout, "c = %s\n", str=print_polynomial(c, degree, x_str));
	free(str);
	math_graph_add_y(c, degree+1, sprintf_ret(name, "Original polynomial coefs"));

	// Eval error
	err = get_polynomial_error_from_points(x, y, p_count, c, degree, NEGMODE);
	fprintf_rl(stdout, "Preoptimisation error %g\n", err);

	for (it=0; it < 60; it++)
	{
		err = get_polynomial_error_from_points(x, y, p_count, c, degree, NEGMODE);
		fprintf_rl(stdout, "%d	error %g\n", it, err);

		// Generate the error curve
		for (i=0; i < p_count; i++)
		{
			a[i] = eval_polynomial(x[i], c, degree);
			ye[i] = y[i] - a[i];
		}

		math_graph_add(x, a, p_count, sprintf_ret(name, "Polynomial curve #%d", it));
		math_graph_add(x, ye, p_count, sprintf_ret(name, "Error curve #%d", it));

		// Transform the error curve
		error_curve_transform(f, p_count, start, end, c, degree, ye, env, env2, b, x, it);

		// Find the optimal k for a+k*b
		k = minmax_find_k(p_count, y, a, b, an, ye, degree);
		fprintf_rl(stdout, "	k = %g\n", k);
		// dampen k
		k = k * 0.1;

		// a' = a + k*b
		for (i=0; i < p_count; i++)
			a[i] += k * b[i];
		math_graph_add(x, a, p_count, sprintf_ret(name, "a' = a + %.4g*b curve #%d", k, it));

		// Fit the curve
		polynomial_fit_on_points_by_dct(a, p_count, start, end, c, degree);
		fprintf_rl(stdout, "	c = %s\n", str=print_polynomial(c, degree, x_str));
		free(str);
	}

	free(x);
	free(y);
	free(a);
	free(an);
	free(ye);
	free(env);
	free(env2);
	free(b);
	free(ce);
}

#ifdef RL_MPFR
void chebyshev_multiplier_by_dct_mpfr(real_t v, real_t *y, int p_count, int id)	// look for the Chebyshev multiplier of degree id
{
	int i;
	real_t x, sum, freq;

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

void polynomial_fit_on_points_by_dct_mpfr(real_t *y, int p_count, real_t start, real_t end, real_t *c, int degree)
{
	int i;
	real_t cm, *cc, *xs;

	r_init(cm);

	xs = r_init_array(2);
	r_zero_array(c, degree+1);

	// x substitution for the shifting
	r_rsub(xs[1], end, start);	// xs[0] = -2.*start/(end-start) - 1.;
	r_rmuld(xs[0], start, -2.);
	r_div(xs[0], xs[1]);
	r_subd(xs[0], 1.);

	r_rddiv(xs[1], 2., xs[1]);	// xs[1] = 2. / (end-start);

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
	r_free(cm);
}

void polynomial_fit_on_function_by_dct_mpfr(void (*f)(real_t,real_t), real_t start, real_t end, real_t *c, int degree)
{
	int i, p_count = 1000;
	real_t *y, v;

	r_init(v);
	y = r_init_array(p_count);

	// Compute the points
	for (i=0; i < p_count; i++)
	{
		// y[i] = f((end-start) * (0.5+0.5*chebyshev_node(p_count, i)) + start);
		chebyshev_node_mpfr(v, p_count, i);
		r_muld(v, 0.5);
		r_addd(v, 0.5);
		r_rsub(y[i], end, start);
		r_fma(v, y[i], v, start);
		f(y[i], v);
	}
	r_free(v);

	// Fit
	polynomial_fit_on_points_by_dct_mpfr(y, p_count, start, end, c, degree);

	r_free_array(&y, degree+1);
}
#endif
