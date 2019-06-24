float gaussian(float x)
{
	return native_exp(-x*x);
}

float erf_fast(float x)
{
	float y, x2 = x*x;

	y = native_sqrt(1.f - native_divide( native_exp(-x2), native_sqrt(x2 + 3.1220878f) - 0.766943f ));

	if (x < 0.f)
		return -y;
	return y;
}
