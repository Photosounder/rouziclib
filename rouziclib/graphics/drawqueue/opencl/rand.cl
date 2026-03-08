uint rand_xsm32(uint x)
{
	x ^= x >> 16;
	x *= 0x21f0aaadu;
	x ^= x >> 15;
	x *= 0x735a2d97u;
	x ^= x >> 15;
	return x;
}

float rand01_to_gaussian_approx(float r)	// max error of the resulting Gaussian distribution: 9.8e-3
{
	r = (r - 0.5f) * 1.9999999f;	// r = ]-0.99999994 , 0.99999994[
	return copysign(0.88622693f * native_sqrt(- native_log(1.f - r*r)), r);		// gives a e^-x^2 distribution, [-3.54 , 3.54]
}
