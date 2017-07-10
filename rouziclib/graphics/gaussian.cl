float gaussian(float x)
{
	return native_exp(-x*x);
}

float erfr(float x)
{
	return 0.5f + 0.5f * erf(x);
}
