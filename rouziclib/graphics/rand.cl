int pow_mod(int base, uint expon, uint mod)
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

uint rand_minstd(uint pos)
{
	return pow_mod(16807, pos, 2147483647);		// bit 0 and bits 11 to 30 are fine, the others get worse towards bit 1
}

uint rand16(uint pos)
{
	return rand_minstd(pos) >> 13 & 0xFFFF;
}

uint rand32(uint pos)
{
	return rand16(pos) << 16 | rand16(pos + 0x80000000);
}

float rand_01(uint pos)	// returns a random float in the range [0 , 1]
{
	return (float) rand32(pos) * 2.3283064370807973754314699618685e-10f;
}

float rand_exc01(uint pos)	// returns a random float in the range (0 , 1]
{
	return (float) (rand32(pos) + 1) * 0.00000000023283064365386962890625f;	// FIXME with single-precision rounding it might return a 0.f anyway
}

float gaussian_rand(uint pos)
{
	float r;		// TODO use rand_minstd(), consider keeping pow_mod's loop short

	pos <<= 1;
	r = native_sqrt(-2.f * native_log(rand_exc01(pos)));
	return r * native_sin(2.f*M_PI_F * rand_01(pos+1)) * M_SQRT1_2_F;	// gives a e^-x^2 distribution
}

float gaussian_rand_approx(uint pos)	// max error: 0.00865 at ±0.772135
{
	float r = ((float) rand_minstd(pos) - 1073741823.f) * 9.313225466e-10f;	// r = ]-1 , 1[
	return copysign(0.8862269254f * native_sqrt(- native_log(1.f - r*r)), r);		// gives a e^-x^2 distribution
}
