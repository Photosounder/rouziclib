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

float sse_sqrtf_raw(float x)		// max relative error 0.894e-7
{
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
	float y = sse_rsqrt_raw(x);
	y = (y*y * x - 3.f) * y * -0.5f;
	return y;
}

float sse_rcpf_newton1(float x)		// max relative error 1.45e-7
{
	float y = sse_rcp_raw(x);
	y = 2.f*y - x * y*y;
	return y;
}
