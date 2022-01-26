float gaussian(float x)
{
	return native_exp(-x*x);	// ~6 FR
}

float erf_fast(float x)
{
	float y, xa = fabs(x);

	// erf(x) ~= 1 - exp(<polynomial>) for x >= 0, max error 1/4249
	// 2 mad, 1 mul, 1 +-, 1 copysign, 1 exp = ~9 FR
	y = ((-0.06388f*xa - 0.66186f)*xa - 1.123613f)*xa;
	y = 1.f - native_exp(y);
	y = copysign(y, x);

	return y;
}

float erf_tri_corner_approx(float x, float y)
{
	float x2, y2, z;

	// 12 mad, 7 mul = 19 FR, max error 1/1802
	x2 = x*x;
	y2 = y*y;
	z = 	((( 5.4082e-6f*y2 - 4.5063e-5f )*x2 + 
		( (7.7728e-6f*y2 - 0.0001990703f)*y2 + 0.00119672f ))*x2 + 
		( (-0.000136548f*y2 + 0.00256433f)*y2 - 0.0146921f ))*x2 + 
		(((1.209847e-5f*y2 - 0.000377797f)*y2 + 0.00542338f)*y2 - 0.04804065f)*y2 + 0.793529f;
	z *= z;		// z^2
	z *= z;		// z^4
	z *= z;		// z^8
	z *= x * y;

	return z;
}
