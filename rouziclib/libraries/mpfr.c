#ifdef RL_MPFR

real *r_init_array(int count)
{
	int i;
	real *a;

	a = calloc(count, sizeof(real));

	for (i=0; i < count; i++)
		r_init(a[i]);

	return a;
}

void r_zero_array(real *a, int count)
{
	int i;

	for (i=0; i < count; i++)
		r_setd(a[i], 0.);
}

void r_free_array(real **a, int count)
{
	int i;

	for (i=0; i < count; i++)
		r_free((*a)[i]);

	free_null(a);
}

void r_flipsign(real y, real x)
{
	int new_sign;

	new_sign = (mpfr_signbit(x) == 0);
	mpfr_setsign(y, x, new_sign, MPFR_RNDN);
}

void r_gaussian(real y, real x)
{
	r_set(y, x);
	r_mul(y, y);
	r_flipsign(y, y);
	r_exp(y, y);
}

#endif
