float sse_rsqrtf_raw(float x)		// max relative error 2.5e-4
{
#ifdef RL_INTEL_INTR
	__m128 v = _mm_load_ss(&x);
	v = _mm_rsqrt_ss(v);
	_mm_store_ss(&x, v);
	return x;
#else
	return 1.f/sqrtf(x);
#endif
}

float sse_sqrtf_raw(float x)		// max relative error 0.894e-7, same results as sqrtf()
{					// (float) sqrt(x) is 0.6e-7
#ifdef RL_INTEL_INTR
	__m128 v = _mm_load_ss(&x);
	v = _mm_sqrt_ss(v);
	_mm_store_ss(&x, v);
	return x;
#else
	return sqrtf(x);
#endif
}

float sse_rcpf_raw(float x)		// max relative error 2.5e-4
{
#ifdef RL_INTEL_INTR
	__m128 v = _mm_load_ss(&x);
	v = _mm_rcp_ss(v);
	_mm_store_ss(&x, v);
	return x;
#else
	return 1.f/x;
#endif
}

float sse_rsqrtf_newton1(float x)	// max relative error 1.6e-7
{
	float y = sse_rsqrtf_raw(x);
	y = (y*y * x - 3.f) * y * -0.5f;
	return y;
}

double sse_rsqrt_newton1(double x)	// max relative error 0.5e-7
{
	double y = sse_rsqrtf_raw(x);
	y = (y*y * x - 3.) * y * -0.5000000247;		// -0.5 is modified to recentre the error
	return y;
}

double sse_rsqrt_newton2(double x)	// max relative error 2.33e-15
{
	double y = sse_rsqrtf_raw(x);
	y = (y*y * x - 3.) * y * -0.5000000247;		// -0.5 is modified to recentre the error
	y = (y*y * x - 3.) * y * -0.5000000000000009;	// error would be 14.9e-15 without these adjustments
	return y;
}

double sse_sqrt_halley(double x)	// max relative error 4.3e-12
{
	double y = x * sse_rsqrtf_raw(x);
	double y2 = y*y;
	y -= 2.*y * (y2 - x) / (3.*y2 + x);
	return y;
}

float sse_rcpf_newton1(float x)		// max relative error 1.45e-7
{
	float y = sse_rcpf_raw(x);
	y = 2.f*y - x * y*y;
	return y;
}
