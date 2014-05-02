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
