#ifndef RL_EXCL_CFFTPACK

#undef pi
#include "orig/fftpack.c"
#undef logical
#define pi RL_PI
#undef complex
#define complex _Complex

void cfft_plan_free(cfft_plan_t *plan)
{
	if (plan==NULL)
		return ;

	free(plan->work);
	free(plan->save);
	memset(plan, 0, sizeof(cfft_plan_t));
}

cfft_plan_t cfft_1D_create_plan(int n)
{
	cfft_plan_t plan={0};
	int ret = 0;

	if (n == 0)
		return plan;

	plan.lensav = n*2 + (int) log2((double) n) + 8;
	plan.save = calloc(plan.lensav, sizeof(fft_real_t));
	plan.lenwork = 2 * n;
	plan.work = calloc(plan.lenwork, sizeof(fft_real_t));
	plan.dim = xyi(n, 1);

	cfft1i_(&n, plan.save, &plan.lensav, &ret);
	if (ret)
	{
		fprintf_rl(stderr, "cfft1i_() returned %d in cfft_1D_create_plan()\n", ret);
		cfft_plan_free(&plan);
	}

	return plan;
}

cfft_plan_t cfft_2D_create_plan(xyi_t dim)
{
	cfft_plan_t plan={0};
	int ret = 0;

	if (is0_xyi(dim))
		return plan;

	plan.lensav = dim.x*2 + (int) log2((double) dim.x) + dim.y*2 + (int) log2((double) dim.y) + 8;
	plan.save = calloc(plan.lensav, sizeof(fft_real_t));
	plan.lenwork = 2 * dim.x * dim.y;
	plan.work = calloc(plan.lenwork, sizeof(fft_real_t));
	plan.dim = dim;

	cfft2i_(&dim.x, &dim.y, plan.save, &plan.lensav, &ret);
	if (ret)
	{
		fprintf_rl(stderr, "cfft2i_() returned %d in cfft_2D_create_plan()\n", ret);
		cfft_plan_free(&plan);
	}

	return plan;
}

int cfft_1D(cfft_plan_t *plan, fft_real_t *data, const int way)
{
	int ret = 0, inc=1;

	if (plan==NULL || data==NULL)
		return -1;

	if (way==0)
		cfft1f_(&plan->dim.x, &inc, data, &plan->dim.x, plan->save, &plan->lensav, plan->work, &plan->lenwork, &ret);
	else
		cfft1b_(&plan->dim.x, &inc, data, &plan->dim.x, plan->save, &plan->lensav, plan->work, &plan->lenwork, &ret);

	return ret;
}

int cfft_2D(cfft_plan_t *plan, fft_real_t *data, const int way)
{
	int ret = 0;

	if (plan==NULL || data==NULL)
		return -1;

	if (way==0)
		cfft2f_(&plan->dim.x, &plan->dim.x, &plan->dim.y, data, plan->save, &plan->lensav, plan->work, &plan->lenwork, &ret);
	else
		cfft2b_(&plan->dim.x, &plan->dim.x, &plan->dim.y, data, plan->save, &plan->lensav, plan->work, &plan->lenwork, &ret);

	return ret;
}

// Processing

fft_complex_t cfft_complex_mul(fft_complex_t a, fft_complex_t b)
{
	fft_complex_t r={0};

	r.r = a.r*b.r - a.i*b.i;
	r.i = a.r*b.i + a.i*b.r;

	return r;
}

void cfft_complex_mul_2D(void *va, void *vb, void *vr, xyi_t dim)
{
	int i, count;
	fft_complex_t *a = va, *b = vb, *r = vr;

	count = mul_x_by_y_xyi(dim);

	for (i=0; i < count; i++)
		r[i] = cfft_complex_mul(a[i], b[i]);
}

void cfft_copy_r2c_pad(fft_real_t *in, size_t in_elemsize, void **pout, size_t *out_as, xyi_t in_dim, xyi_t out_dim)	// makes a 0-padded complex copy
{
	xyi_t ip;
	fft_complex_t *out;

	alloc_enough(pout, mul_x_by_y_xyi(out_dim), out_as, sizeof(fft_complex_t), 1.);
	out = *pout;
	memset(out, 0, mul_x_by_y_xyi(out_dim) * sizeof(fft_complex_t));

	if (in_elemsize == sizeof(float))
		for (ip.y=0; ip.y < in_dim.y; ip.y++)
			for (ip.x=0; ip.x < in_dim.x; ip.x++)
				out[ip.y*out_dim.x + ip.x].r = ((float *) in)[ip.y*in_dim.x + ip.x];
	else
		for (ip.y=0; ip.y < in_dim.y; ip.y++)
			for (ip.x=0; ip.x < in_dim.x; ip.x++)
				out[ip.y*out_dim.x + ip.x].r = in[ip.y*in_dim.x + ip.x];
}

void cfft_copy_c2r(fft_real_t *in, fft_real_t *out, xyi_t dim)
{
	int i, count = mul_x_by_y_xyi(dim);
	fft_complex_t *cin = in;

	for (i=0; i < count; i++)
		out[i] = cin[i].r;
}

void cfft_1D_c2c_fft(cfft_plan_t *plan, fft_real_t *array, int n)	// when the input is already alloced, padded and complex
{
	if (equal_xyi(plan->dim, xyi(n, 1))==0)		// if we must recreate the plan
	{
		cfft_plan_free(plan);
		*plan = cfft_1D_create_plan(n);
	}

	cfft_1D(plan, array, 0);
}

void cfft_1D_r2c_padded_fft(cfft_plan_t *plan, fft_real_t *in, size_t in_elemsize, void **pout, size_t *out_as, int in_n, int out_n)	// every arg besides in and *_n can be uninitialised
{
	xyi_t in_dim, out_dim;

	in_dim = xyi(in_n, 1);
	out_dim = xyi(out_n, 1);

	cfft_copy_r2c_pad(in, in_elemsize, pout, out_as, in_dim, out_dim);	// 2D real -> 2D complex and padded

	cfft_1D_c2c_fft(plan, *pout, out_n);
}

void cfft_2D_r2c_padded_fft(cfft_plan_t *plan, fft_real_t *in, size_t in_elemsize, void **pout, size_t *out_as, xyi_t in_dim, xyi_t out_dim)	// every arg besides in and *_dim can be uninitialised
{
	cfft_copy_r2c_pad(in, in_elemsize, pout, out_as, in_dim, out_dim);	// 2D real -> 2D complex and padded

	if (equal_xyi(plan->dim, out_dim)==0)			// if we must recreate the plan
	{
		cfft_plan_free(plan);
		*plan = cfft_2D_create_plan(out_dim);
	}

	cfft_2D(plan, *pout, 0);
}

void cfft_1D_c2r_full_ifft(cfft_plan_t *plan, fft_real_t *in, fft_real_t *out, int n)	// the IFFT is still in-place for in
{
	xyi_t dim = xyi(n, 1);

	if (equal_xyi(plan->dim, dim)==0)			// if we must recreate the plan
	{
		cfft_plan_free(plan);
		*plan = cfft_1D_create_plan(n);
	}

	cfft_1D(plan, in, 1);

	cfft_copy_c2r(in, out, dim);
}

void cfft_2D_c2r_full_ifft(cfft_plan_t *plan, fft_real_t *in, fft_real_t *out, xyi_t dim)	// the IFFT is still in-place for in
{
	if (equal_xyi(plan->dim, dim)==0)			// if we must recreate the plan
	{
		cfft_plan_free(plan);
		*plan = cfft_2D_create_plan(dim);
	}

	cfft_2D(plan, in, 1);

	cfft_copy_c2r(in, out, dim);
}



void cfft_test()
{
	int i;
	xyi_t dim;
	cfft_plan_t plan;
	fft_real_t *data;

	dim = xyi(1024, 1024);
	data = calloc(mul_x_by_y_xyi(dim), 2*sizeof(fft_real_t));
	for (i=0; i<mul_x_by_y_xyi(dim); i++)
		data[i*2] = gaussian_rand();

	plan = cfft_2D_create_plan(dim);

	uint32_t td=0;
	get_time_diff(&td);
	cfft_2D(&plan, data, 0);
	fprintf_rl(stdout, "cfft_2D() took %d ms\n\n", get_time_diff(&td));

	cfft_plan_free(&plan);
}

#endif
