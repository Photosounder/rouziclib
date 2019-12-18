float u32_as_float(const uint32_t i)
{
	return *((float *) &i);
}

double u64_as_double(const uint64_t i)
{
	return *((double *) &i);
}

uint32_t float_as_u32(const float f)
{
	return *((uint32_t *) &f);
}

uint64_t double_as_u64(const double f)
{
	return *((uint64_t *) &f);
}

int float_get_exponent(const float f)
{
	uint32_t i = *((uint32_t *) &f);
	int e;

	e = (i >> 23) & 0xFF;
	e -= 127;

	return e;
}

int double_get_exponent(const double f)
{
	uint64_t i = *((uint64_t *) &f);
	int e;

	e = (i >> 52) & 0x07FF;
	e -= 1023;

	return e;
}

uint32_t float_get_mantissa(const float f)
{
	return float_as_u32(f) & 0x007FFFFF;
}

uint64_t double_get_mantissa(const double f)
{
	return double_as_u64(f) & 0x000FFFFFFFFFFFFF;
}

float get_fractional_partf(float f)	// gets the fractional part of the number in the [0 , 1[ range, conserves the sign
{
	return f - truncf(f);
}

double get_fractional_part(double f)
{
	return f - trunc(f);
}

double double_add_ulp(double x, int ulp)	// add an integer to the mantissa of a double
{
	uint64_t xi = *((uint64_t *) &x);
	double r;

	xi += ulp;
	r = *((double *) &xi);

	return r;
}

int64_t double_diff_ulp(double a, double b)	// gives the (signed) difference between two doubles in ulp
{
	uint64_t ai = *((uint64_t *) &a);
	uint64_t bi = *((uint64_t *) &b);

	return ai - bi;
}

double double_increment_minulp(double v0, double inc)	// guarantees a minimal incrementation if the increment is too small to increment anything
{
	double v1;

	v1 = v0 + inc;

	if (v1==v0)
		v1 = double_add_ulp(v0, sign(inc));

	return v1;
}
