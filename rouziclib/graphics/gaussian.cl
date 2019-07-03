float gaussian(float x)
{
	return native_exp(-x*x);
}

float erf_fast(float x)
{
	float y, xa = fabs(x);

	// erf(x) ~= 1 - 1/( x*(x*(x*0.0038004543 + 0.020338153) + 0.03533611) + 1.0000062 )^32 for x >= 0, max error 0.35e-3
	y = xa*(xa*(xa * 0.0038004543f + 0.020338153f) + 0.03533611f) + 1.0000062f;

	// y = y^32
	y = y*y;
	y = y*y;
	y = y*y;
	y = y*y;
	y = y*y;

	y = 1.f - native_recip(y);

	if (x < 0.f)
		return -y;
	return y;
}
