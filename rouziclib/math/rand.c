uint32_t rand32()
{
	return (rand()&0x7FE0)<<17 | (rand()&0x7FF0)<<7 | (rand()&0x7FF0)>>4;
}

double randrange(double low, double high)
{
	double range;

	range = high-low;

	if (range <= 0.)
		return 0.;

	return range * (double)rand32() / 4294967295. + low;
}

double gaussian_rand()
{
	double v1, v2, s;

	do
	{
		v1 = randrange(-1., 1.);
		v2 = randrange(-1., 1.);
		s = v1*v1 + v2*v2;
	}
	while (s >= 1.);	// clips points inside the unit square but outside the unit circle
	
	return sqrt(-2. * log(s) / s) * v1;
	//y = sqrt(-2. * log(s) / s) * v2;
}

xy_t gaussian_rand_xy()
{
	double v1, v2, s;

	do
	{
		v1 = randrange(-1., 1.);
		v2 = randrange(-1., 1.);
		s = v1*v1 + v2*v2;
	}
	while (s >= 1.);	// clips points inside the unit square but outside the unit circle
	
	return xy( sqrt(-2. * log(s) / s) * v1 , sqrt(-2. * log(s) / s) * v2 );
}

// adapted from rand.cl
int pow_mod(int base, uint32_t expon, uint32_t mod)
{
	int x = 1, power = base % mod;

	for (; expon > 0; expon >>= 1)
	{
		if (expon & 1)
			x = (x * power) % mod;

		power = (power * power) % mod;
	}

	return x;
}

uint32_t rand_minstd(uint32_t pos)
{
	return pow_mod(16807, pos, 2147483647);		// bit 0 and bits 11 to 30 are fine, the others get worse towards bit 1
}

uint32_t rand_minstd16(uint32_t pos)
{
	return rand_minstd(pos) >> 13 & 0xFFFF;
}

uint32_t rand_minstd32(uint32_t pos)
{
	return rand_minstd16(pos) << 16 | rand_minstd16(pos + 0x80000000);
}

double rand_minstd_01(uint32_t pos)	// returns a random double in the range [0 , 1]
{
	return (double) rand_minstd32(pos) * 2.3283064370807973754314699618685e-10;
}

double rand_minstd_exc01(uint32_t pos)	// returns a random double in the range (0 , 1]
{
	return (double) (rand_minstd32(pos) + 1) * 0.00000000023283064365386962890625;
}

double gaussian_rand_minstd(uint32_t pos)
{
	double r;
	const double m_sqrt1_2 = 1. / sqrt(2.);

	pos <<= 1;
	r = sqrt(-2. * log(rand_minstd_exc01(pos)));
	return r * sin(2.*pi * rand_minstd_01(pos+1)) * m_sqrt1_2;	// gives a e^-x^2 distribution
}

double gaussian_rand_minstd_approx(uint32_t pos)					// max error: 0.00865 at ±0.772135
{
	double r = ((double) rand_minstd(pos) - 1073741823.) * 9.313225466e-10;		// r = ]-1 , 1[
	return copysign(0.8862269254 * sqrt(- log(1. - r*r)), r);			// gives a e^-x^2 distribution
}
