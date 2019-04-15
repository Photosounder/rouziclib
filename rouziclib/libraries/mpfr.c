#ifdef RL_MPFR

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
